#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>

// variables used for P7
int sem_id7;
pthread_cond_t th3_end = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex3;
pthread_cond_t th3_wait_th1_to_start;
pthread_cond_t th1_wait_th3_to_end;

// acolesa: variabile care indica conditiile asteptate
int th_1_started = 0;
int th_3_ended = 0;

// variables used for P5
int sem_id5;
int th11_s = 0;
int th11_e = 0;
int blocked = 0;
pthread_mutex_t mutex5;
pthread_cond_t th11_end = PTHREAD_COND_INITIALIZER;
pthread_cond_t running_5_th = PTHREAD_COND_INITIALIZER;
int rt = 0;

// variables used for P4
int sem_id4;


void P(int sem_id, int sem_nr)
{
    struct sembuf op = {sem_nr, -1, 0};
    semop(sem_id, &op, 1);
}

void V(int sem_id, int sem_nr)
{
    struct sembuf op = {sem_nr, +1, 0};
    semop(sem_id, &op, 1);
}

void thread_function_7(void* arg)
{
    int thread_no = *(int*)arg;

    //thread 7.5 waits for 4.5 to finish
    if( thread_no == 5)
    {
        P(sem_id4, 0);
    }

    // acolesa: am mutat inainte de info(BEGIN...) acest test, pentru ca intai trebuie sa verifici daca poti face BEGIN
    // thread 1 waits for thread 3 to finish
    if( thread_no == 3) // acolesa: verificarea e pentru T7.3, nu pentru T7.1 pentru ca T7.3 NU POATE incepe PANA NU a inceput T7.1 (asa se traduce "Thread T7.1 must start before T7.3")
    {
        pthread_mutex_lock(&mutex3);
        // acolesa: NICIODATA wait fara un if / while in fata; astepti doar daca o conditia VERIFICATA (nu doar presupusa) nu e indeplinita
        // acolesa: aici conditie e "daca T7.1 nu a ponit inca"; daca nu a pornit, T7.3 nu poate inca incepe
        while (th_1_started == 0)
            pthread_cond_wait(&th3_wait_th1_to_start, &mutex3);

        pthread_mutex_unlock(&mutex3);
    }

    // thread start
    info(BEGIN, 7, thread_no);

    // acolesa: th 1 trebuie sa-l anute pe th 3 ca a el (T1) inceput si ca poate incepe si el (T3)
    if (thread_no == 1)
    {
        pthread_mutex_lock(&mutex3);
        th_1_started = 1;
        pthread_cond_signal(&th3_wait_th1_to_start);
        pthread_mutex_unlock(&mutex3);
    }

    // acolesa: mutat verificarea inainte de info(END)!!! Nu are logica dupa ce faci info(END)
    if( thread_no == 1) // pentru T1, nu pentru T3 !
    {
        pthread_mutex_lock(&mutex3);
        // acolesa: NICIODATA wait FARA if /while !!!
        while (th_3_ended == 0)
            pthread_cond_wait(&th1_wait_th3_to_end, &mutex3);
        pthread_mutex_unlock(&mutex3);
    }

    // thread end
    info(END, 7, thread_no);

    // thread 3 signals thread 1 that it has finished
    if( thread_no == 3)
    {
        pthread_mutex_lock(&mutex3);
        th_3_ended = 1;   // acolesa
        pthread_cond_signal(&th1_wait_th3_to_end); // acolesa: semnalizeaza pe T1 ca T3 s-a terminat
        pthread_mutex_unlock(&mutex3);
    }

    // 7.5 signals 4.4 that it has finished
    if(thread_no == 5)
    {
        V(sem_id7, 0);
    }
}

void thread_function_5(void* arg)
{
    int thread_no = *(int*)arg;

    P(sem_id5, 0);



    info(BEGIN, 5, thread_no);
    // th 11 start
    if(thread_no == 11)
    {
        pthread_mutex_lock(&mutex5);
        th11_s = 1;
        pthread_mutex_unlock(&mutex5);
    }
    // count the number of threads which have started
    pthread_mutex_lock(&mutex5);
    rt++;
    pthread_mutex_unlock(&mutex5);

    // if thread 11 started and not ended, the threads in the tuple will wait
    if(thread_no != 11  && th11_s == 1 && th11_e == 0)
    {
        // count nb of threads blocked waiting for th 11 to end
        // when there are 5, th 11 can finish
        pthread_mutex_lock(&mutex5);
        blocked++;
        if(blocked == 5)
            pthread_cond_signal(&running_5_th);
        pthread_mutex_unlock(&mutex5);

        // the threads wait for th 11 to end
        while(th11_e == 0)
        {
            pthread_mutex_lock(&mutex5);
            pthread_cond_wait(&th11_end, &mutex5);
            pthread_mutex_unlock(&mutex5);
        }
    }

    // the last 5 threads wait for th 11 in case it has not ended yet
    if(rt > 30 && thread_no != 11)
    {
        blocked = 5;
        while(th11_e == 0)
        {
            pthread_mutex_lock(&mutex5);
            pthread_cond_wait(&th11_end, &mutex5);
            pthread_mutex_unlock(&mutex5);
        }
    }

    // th 11 waits till 5 threads run in order to finish
    if(thread_no == 11)
    {
        while(blocked != 5)
        {
            pthread_mutex_lock(&mutex5);
            pthread_cond_wait(&running_5_th, &mutex5);
            pthread_mutex_unlock(&mutex5);
        }
    }

    info(END, 5, thread_no);

    // th 11 end
    if(thread_no == 11)
    {
        pthread_mutex_lock(&mutex5);
        th11_e = 1;
        pthread_cond_broadcast(&th11_end);
        pthread_mutex_unlock(&mutex5);
    }

    V(sem_id5, 0);
}

void thread_function_4(void* arg)
{
    int thread_no = *(int*)arg;

    // 4.4 waits for 7.5 to end in order to start
    if( thread_no == 4)
    {
        P(sem_id7, 0);
    }

    // thread start
    info(BEGIN, 4, thread_no);

    // thread end
    info(END, 4, thread_no);

    // 4.5 signals 7.5 it has ended
    if(thread_no == 5)
    {
        V(sem_id4, 0);
    }

}


int main()
{
    int pid2, pid3, pid4, pid5, pid6, pid7, pid8, pid9;

    init();
    // create P1
    info(BEGIN, 1, 0);

    // create P2
    pid2 = fork();
    if(pid2 == 0)
    {
        // start P2
        info(BEGIN, 2, 0);

        // create P3
        if((pid3 = fork()) == 0)
        {
            // start P3
            info(BEGIN, 3, 0);

            // create P4
            pid4 = fork();
            if(pid4 == 0)
            {
                // start P4
                info(BEGIN, 4, 0);

                // P4 creates 6 threads
                pthread_t* threads4 = (pthread_t*)malloc(sizeof(pthread_t) * 6);
                int* th_no_4 = (int*)malloc(sizeof(int) * 6);

                // initialize semaphores for P4 and P7
                sem_id4 = semget(1001, 1, IPC_CREAT | 0600);
                semctl(sem_id4, 0, SETVAL, 0);
                sem_id7 = semget(1002, 1, IPC_CREAT | 0600);
                semctl(sem_id7, 0, SETVAL, 0);
                //semctl(sem_id7, 1, SETVAL, 0);

                for(int i = 0; i <= 5 ; i++)
                {
                    th_no_4[i] = i+1;
                    pthread_create(&threads4[i], NULL, (void* (*) (void*)) thread_function_4,  &th_no_4[i] );
                }
                // create P5
                pid5 = fork();
                if (pid5 == 0)
                {
                    // start P5
                    info(BEGIN, 5, 0);

                    pthread_mutex_init(&mutex5, NULL);

                    // P5 creates 36 threads
                    pthread_t* threads5 = (pthread_t*)malloc(sizeof(pthread_t) * 36);
                    int* th_no_5 = (int*)malloc(sizeof(int) * 36);

                    // initialize semaphores for P5
                    sem_id5 = semget(1000, 4, IPC_CREAT | 0600);
                    semctl(sem_id5, 0, SETVAL, 6);
                    semctl(sem_id5, 1, SETVAL, 0);
                    semctl(sem_id5, 2, SETVAL, 0);
                    semctl(sem_id5, 3, SETVAL, 0);

                    for(int i = 0; i <= 35; i++)
                    {
                        th_no_5[i] = i+1;
                        pthread_create(&threads5[i], NULL, (void* (*) (void*)) thread_function_5,  &th_no_5[i] );
                    }

                    for(int i = 0; i <=  35; i++)
                    {
                        pthread_join(threads5[i], NULL);
                    }

                    // create P6
                    pid6 = fork();
                    if(pid6 == 0)
                    {
                        // start P6
                        info(BEGIN, 6, 0);

                        //create P7
                        if((pid7 = fork()) == 0)
                        {
                            // start P7
                            info(BEGIN, 7, 0);

                            pthread_mutex_init(&mutex3, NULL);
                            pthread_cond_init(&th3_wait_th1_to_start, NULL);
                            pthread_cond_init(&th1_wait_th3_to_end, NULL);

                            // P7 creates 5 threads
                            pthread_t* threads7 = (pthread_t*)malloc(sizeof(pthread_t) * 5);
                            int* th_no_7 = (int*)malloc(sizeof(int) * 5);

                            for(int i = 4; i >= 0; i--)
                            {
                                th_no_7[i] = i+1;
                                pthread_create(&threads7[i], NULL, (void* (*) (void*)) thread_function_7,  &th_no_7[i] );
                            }

                            for(int i = 4; i >= 0; i--)
                            {
                                pthread_join(threads7[i], NULL);
                            }

                            free(threads7);
                            free(th_no_7);
                            // end P7
                            info(END, 7, 0);
                        }
                        // create P8
                        else
                        {
                            if((pid8 = fork()) == 0)
                            {
                                // start P8
                                info(BEGIN, 8, 0);

                                // end P7
                                info(END, 8, 0);

                            }
                            else
                            {
                                waitpid(pid7, NULL, 0);
                                waitpid(pid8, NULL, 0);
                                info(END, 6, 0);
                            }
                        }
                    }

                    else
                    {
                        waitpid(pid6, NULL, 0);
                        info(END, 5, 0);
                    }



                    free(threads5);
                    free(th_no_5);
                }

                else
                {
                    waitpid(pid5, NULL, 0);
                    info(END, 4, 0);
                }
                for(int i = 0; i <= 5; i++)
                {
                    pthread_join(threads4[i], NULL);
                }

                free(threads4);
                free(th_no_4);
            }

            else
            {
                waitpid(pid4, NULL, 0);
                info(END, 3, 0);
            }
        }
        // create P9
        else
        {
            if((pid9 = fork()) == 0)
            {
                // start P9
                info(BEGIN, 9, 0);

                // end P9
                info(END, 9, 0);
            }

            else
            {
                waitpid(pid3, NULL, 0);
                info(END, 2, 0);
            }
        }
    }
    else
    {
        waitpid(pid2, NULL, 0);
        info(END, 1, 0);
    }

    return 0;
}
