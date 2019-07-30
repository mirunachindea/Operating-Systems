#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
int fd_req;
int fd_resp;
int shm_id;
int shm_key;
char *shared_char;
char *file;
int file_size;

void write_error(int msg_size, char *msg)
{
    char* resp_msg = (char*)malloc(msg_size * sizeof(char));
    strcpy(resp_msg, msg);

    char* err = (char*)malloc(5 * sizeof(char));
    strcpy(err,"ERROR");

    write(fd_resp, &msg_size, 1);
    write(fd_resp, resp_msg, msg_size);

    char size2 = 5;

    write(fd_resp, &size2, 1);
    write(fd_resp, err, 5);

    free(resp_msg);
    free(err);

}

void write_success(int msg_size, char *msg)
{
    char* resp_msg = (char*)malloc(msg_size * sizeof(char));
    strcpy(resp_msg, msg);

    char* succ = (char*)malloc(7 * sizeof(char));
    strcpy(succ,"SUCCESS");

    write(fd_resp, &msg_size, 1);
    write(fd_resp, resp_msg, msg_size);

    char size2 = 7;

    write(fd_resp, &size2, 1);
    write(fd_resp, succ, 7);

    free(resp_msg);
    free(succ);
}

int create_response_pipe()
{
    if(mkfifo("RESP_PIPE_27829", 0600) < 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return -1;
    }

    return 1;
}

int open_response_pipe()
{
    fd_resp = open("RESP_PIPE_27829", O_WRONLY);

    if(fd_resp < 0)
    {
        return -1;
    }

    return 1;
}

int open_request_pipe()
{
    fd_req = open("REQ_PIPE_27829", O_RDONLY);

    if(fd_req < 0)
    {
        printf("ERROR\ncannot open the request pipe\n");
        return -1;
    }

    return 1;
}

int create_connection()
{
    char size = 7;
    char *connect = (char*)malloc(8 * sizeof(char));
    strcpy(connect, "CONNECT");

    if(write(fd_resp, &size, 1) < 0)
    {
        return -1;
    }

    if(write(fd_resp, connect, 7) < 0)
    {
        return -1;
    }

    free(connect);

    printf("SUCCESS\n");
    return 1;
}

void ping()
{
    char size = 4;
    char *ping = (char*)malloc(5 * sizeof(char));
    strcpy(ping, "PING");
    char *pong = (char*)malloc(5 * sizeof(char));
    strcpy(pong, "PONG");
    unsigned int n = 27829;

    write(fd_resp, &size, 1);
    write(fd_resp, ping, 4);
    write(fd_resp, &size, 1);
    write(fd_resp, pong, 4);
    write(fd_resp, &n, sizeof(unsigned int));

    free(ping);
    free(pong);
}

void create_shm()
{
    // read size
    unsigned int size;
    read(fd_req, &size, sizeof(unsigned int));

    // create shared memory
    shm_key = 17081;
    shm_id = shmget(shm_key, sizeof(char) * size, IPC_CREAT | 0644);

    // if cannot create shared memory
    if(shm_id < 0)
    {
        write_error(10, "CREATE_SHM");
    }

    else
    {
        shared_char = (char*)shmat(shm_id, NULL, 0);

        // if cannot attach shared memory
        if(shared_char == (void*)-1)
        {
            write_error(10, "CREATE_SHM");
        }

        // success
        else
        {
            write_success(10, "CREATE_SHM");
        }
    }
}

void write_to_shm()
{
    // read offset
    unsigned int offset;
    read(fd_req, &offset, sizeof(unsigned int));

    unsigned int value;
    read(fd_req, &value, sizeof(unsigned int));

    // error
    if(offset < 0 || offset >= 4494150 || (offset + sizeof(unsigned int)) > 4494150)
    {
        write_error(12, "WRITE_TO_SHM");
    }
    else
    {
        shared_char[offset + 0] = value & 0xFF;
        shared_char[offset + 1] = (value >> 8) & 0xFF;
        shared_char[offset + 2] = (value >> 16) & 0xFF;
        shared_char[offset + 3] = (value >> 24) & 0xFF;

        write_success(12, "WRITE_TO_SHM");
    }

}

void exit_req()
{
    close(fd_resp);
    close(fd_req);
}

void map_file()
{
    char *file_name;
    char size;
    read(fd_req, &size, 1);

    file_name = (char*)malloc(sizeof(char) * ((int)size+1));
    read(fd_req, file_name, (int)size);

    int fd_map = open(file_name, O_RDONLY);

    // can't open file
    if(fd_map < 0)
    {
        write_error(8, "MAP_FILE");
    }

    else
    {
        file = (char*)mmap(NULL, sizeof(char) * 50000, PROT_READ, MAP_PRIVATE, fd_map, 0);
        file_size = lseek(fd_map, 0, SEEK_END);

        // can't map file
        if(file == (void*)-1)
        {
            write_error(8, "MAP_FILE");
        }
        else
        {
            write_success(8, "MAP_FILE");
        }
    }

    free(file_name);
}

void read_file_offset()
{
    unsigned int offset;
    read(fd_req, &offset, sizeof(unsigned int));

    unsigned int no_of_bytes;
    read(fd_req, &no_of_bytes, sizeof(unsigned int));

    if(shared_char == (void*)-1 || file == (void*)-1 ||  file_size < (offset+ no_of_bytes))
    {
        write_error(21, "READ_FROM_FILE_OFFSET");
    }
    else
    {
        char *aux = (char*)malloc(sizeof(char) * no_of_bytes);

        for(int i = 0; i < no_of_bytes; i++)
        {
            aux[i] = file[offset + i];
        }


        for(int i = 0; i < no_of_bytes; i++)
        {
            shared_char[i] = aux[i];
        }

        write_success(21, "READ_FROM_FILE_OFFSET");

        free(aux);

    }

}

void read_file_section()
{
    unsigned int section_no;
    read(fd_req, &section_no, sizeof(unsigned int));

    unsigned int offset;
    read(fd_req, &offset, sizeof(unsigned int));

    unsigned int no_of_bytes;
    read(fd_req, &no_of_bytes, sizeof(unsigned int));

    if(shared_char == (void*)-1 || file == (void*)-1 || section_no > 12 || section_no <= 0)
    {
        write_error(22, "READ_FROM_FILE_SECTION");
    }
    else
    {
        int header_size;
        memcpy(&header_size, file+(file_size-6), 2);

        int no_of_sections;
        memcpy(&no_of_sections, file+(file_size - header_size + 1), 1);

        char *sect_name = (char*)malloc(sizeof(char) * 10);
        memcpy(sect_name, file+(file_size - header_size + 2 + 20 * (section_no - 1 )), 10);

        int sect_offset;
        memcpy(&sect_offset, file+(file_size - header_size + 2 + 20 * (section_no - 1 ) + 12), 4);

        int sect_size;
        memcpy(&sect_size, file+(file_size - header_size + 2 + 20 * (section_no - 1 ) + 16), 4);

        char *aux = (char*)malloc(sizeof(char) * no_of_bytes);
        memcpy(aux, file + sect_offset + offset, no_of_bytes);

        for(int i = 0; i < no_of_bytes; i++)
        {
            shared_char[i] = aux[i];
        }

        write_success(22, "READ_FROM_FILE_SECTION");

        free(sect_name);
        free(aux);
    }

}

void read_logical_space_offset()
{
    unsigned int logical_offset;
    read(fd_req, &logical_offset, sizeof(unsigned int));

    unsigned int no_of_bytes;
    read(fd_req, &no_of_bytes, sizeof(unsigned int));

    int *sect_offset = (int*)malloc(20 * sizeof(int));
    int *sect_size = (int*)malloc(20 * sizeof(int));
    int *start = (int*)malloc(20 * sizeof(int));
    int *end = (int*)malloc(20 * sizeof(int));
    int page;
    int read_addr;
    int success = 0;

    start[0] = 0;
    page = logical_offset / 5120;

    int header_size;
    memcpy(&header_size, file+(file_size-6), 2);

    int no_of_sections = 0;
    memcpy(&no_of_sections, file+ (file_size - header_size + 1 ), 1);

    for(int i = 0; i < no_of_sections; i++)
    {
        char *sect_name = (char*)malloc(sizeof(char) * 10);
        memcpy(sect_name, file + file_size - header_size + 2 + 20 * i, 10);

        memcpy(&sect_offset[i], file + file_size - header_size + 2 + 20 * i + 12, 4);

        memcpy(&sect_size[i], file + file_size - header_size + 2 + 20 * i + 16, 4);

        if (i != 0)
            start[i] = end[i-1] + 1;

        end[i] = sect_size[i] / 5120 + start[i];

        if(sect_size[i] % 5120 == 0)
            end[i] --;

        if(page >= start[i] && page <= end[i])
        {
            read_addr = sect_offset[i] + logical_offset - (start[i] * 5120);

            for(int j = 0; j < no_of_bytes; j++)
            {
                shared_char[j] = file[j + read_addr];
            }

            success = 1;
        }
    }

    if(success == 1)
        write_success(30, "READ_FROM_LOGICAL_SPACE_OFFSET");
    else
        write_error(30, "READ_FROM_LOGICAL_SPACE_OFFSET");

    free(sect_offset);
    free(sect_size);
    free(start);
    free(end);
}

int main(int argc, char **argv)
{
    if (create_response_pipe() < 0)
        exit(1);

    if(open_request_pipe() < 0)
        exit(1);

    if(open_response_pipe() < 0)
        exit(1);

    if(create_connection() < 0)
        exit(1);

    char *buff_rd;
    int* req_size = (int*)malloc(1024 * sizeof(int));

    int i = 0;

    while(1)
    {
        read(fd_req, &req_size[i], 1);
        buff_rd = (char*)malloc(sizeof(char) * (req_size[i]+1));
        read(fd_req, buff_rd, req_size[i]);
        buff_rd[req_size[i]] = '\0';

        if(strcmp(buff_rd, "PING") == 0)
        {
            ping();
        }

        else if(strcmp(buff_rd, "CREATE_SHM") == 0)
        {
            create_shm();
        }

        else if(strcmp(buff_rd, "WRITE_TO_SHM") == 0)
        {
            write_to_shm();
        }

        else if(strcmp(buff_rd, "MAP_FILE") == 0)
        {
            map_file();
        }

        else if(strcmp(buff_rd, "READ_FROM_FILE_OFFSET") == 0)
        {
            read_file_offset();
        }

        else if(strcmp(buff_rd, "READ_FROM_FILE_SECTION") == 0)
        {
            read_file_section();
        }

        else if(strcmp(buff_rd, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            read_logical_space_offset();
        }

        else if(strcmp(buff_rd, "EXIT") == 0)
        {
            exit_req();
            return 0;
        }

        i++;
        free(buff_rd);

    }
    //free(buff_rd);
    return 0;
}
