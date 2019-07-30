#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

/**
* function for listing the contents of a directory
**/
void listDir(char *dirName)
{

    DIR* dir;
    struct dirent *dirEntry;
    dir = opendir(dirName);

    if (dir == 0)
    {
        printf("ERROR\ninvalid directory path");
        return;
    }
    else
    {
        printf("SUCCESS\n");
        while ((dirEntry=readdir(dir)) != 0)
        {
            if( strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0)
                printf("%s/%s\n", dirName, dirEntry->d_name);
        }
    }

    closedir(dir);
}

/**
* function for listing the contents of a directory recursively
**/
void listDirRec(const char *dirName, int first)
{

    DIR *dir;
    struct dirent *dirEntry;
    char path[1024];

    if (!(dir = opendir(dirName)))
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    while ((dirEntry = readdir(dir)) != NULL)
    {
        if (first == 1)
        {
            printf("SUCCESS\n");
            first = 0;
        }
        if (dirEntry->d_type == DT_DIR)
        {
            if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
            printf("%s/%s\n",dirName, dirEntry->d_name);
            listDirRec(path, 0);
        }
        else
        {
            printf("%s/%s\n",dirName, dirEntry->d_name);
        }
    }

    closedir(dir);
}

/**
* function for listing the files with owner write permission recursively
**/
void listPermWriteRec(char *dirName, int first)
{

    DIR *dir;
    struct dirent *dirEntry;
    char path[1024];
    struct stat fileMetadata;

    if (!(dir = opendir(dirName)))
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    while ((dirEntry = readdir(dir)) != NULL)
    {
        if (first == 1)
        {
            printf("SUCCESS\n");
            first = 0;
        }
        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
            if( stat(path, &fileMetadata) < 0)
            {
                printf("ERROR\ninvalid directory path");
                return;
            }
            /// if it has write owner permission
            if( fileMetadata.st_mode & S_IWUSR )
            {
                /// if it is a directory
                if (dirEntry->d_type == DT_DIR)
                {
                    printf("%s\n", path);
                    listPermWriteRec(path, 0);
                }
                /// if it is a file
                else
                {
                    printf("%s\n", path);
                }
            }
            ///if it doesn't have write permission
            else if (dirEntry->d_type == DT_DIR)
            {
                listPermWriteRec(path, 0);
            }
        }
    }

    closedir(dir);
}

/**
* function for listing the files with owner write permission
**/
void listPermWrite(char *dirName)
{

    DIR* dir;
    struct dirent *dirEntry;
    dir = opendir(dirName);
    struct stat fileMetadata;
    char path[1024];

    if (dir == 0)
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    else
    {
        printf("SUCCESS\n");
        while ((dirEntry=readdir(dir)) != 0)
        {
            if( strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0)
            {
                snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
                if( stat(path, &fileMetadata) < 0)
                {
                    printf("ERROR\ninvalid directory path");
                    return;
                }
                if( fileMetadata.st_mode & S_IWUSR )
                    printf("%s\n", path);
            }
        }
    }
    closedir(dir);
}

/**
* function for listing the files with size smaller than a given size
**/
void listSizeSmaller(char *dirName, int size)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat fileMetadata;
    char path[1024];
    dir = opendir(dirName);

    if (dir == 0)
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    else
    {
        printf("SUCCESS\n");
        while ((dirEntry=readdir(dir)) != 0)
        {
            if( strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0 )
            {
                snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
                if( stat(path, &fileMetadata) < 0)
                {
                    printf("ERROR\ninvalid directory path");
                    return;
                }
                if(dirEntry->d_type != DT_DIR)
                {
                    if(fileMetadata.st_size < size)
                    {
                        printf("%s\n",path);
                    }
                }
            }
        }
    }

    closedir(dir);
}

/**
* function for listing the files with size smaller than a given size recursively
**/
void listSizeSmallerRec(const char *dirName, int size, int first)
{
    DIR *dir;
    struct dirent *dirEntry;
    char path[1024];
    struct stat fileMetadata;

    if (!(dir = opendir(dirName)))
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    while ((dirEntry = readdir(dir)) != NULL)
    {
        if (first == 1)
        {
            printf("SUCCESS\n");
            first = 0;
        }
        if( strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0 )
        {
            snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
            if( stat(path, &fileMetadata) < 0)
            {
                printf("ERROR\ninvalid directory path");
                return;
            }
            if(dirEntry->d_type != DT_DIR)
            {
                if(fileMetadata.st_size < size)
                {
                    printf("%s\n",path);
                }
            }
            else
            {
                listSizeSmallerRec(path, size, 0);
            }
        }
    }

    closedir(dir);
}

/**
* function for parsing section files
**/
void parse(char *fileName)
{

    int fd;
    char buf_magic[4];
    short buf_size;
    char buf_version;
    char buf_nbofsections;
    char buf_sectname[10];
    char buf_sectnames[11][11];
    short buf_secttype;
    short buf_secttypes[11];
    long buf_sectoffset;
    long buf_sectsize;
    long buf_sectsizes[11];
    int i;

    if( (fd = open(fileName, O_RDONLY)) < 0)
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    lseek(fd, -4, SEEK_END);
    read(fd, buf_magic, 4);

    if (strcmp(buf_magic, "mJcq") != 0 )
    {
        printf("ERROR\nwrong magic");
        return;
    }

    lseek(fd, -6, SEEK_END);
    read(fd, &buf_size,2);
    lseek(fd, -buf_size, SEEK_END);
    read(fd, &buf_version, 1);

    if( (int)buf_version < 77 || (int)buf_version > 110)
    {
        printf("ERROR\nwrong version");
        return;
    }

    read(fd, &buf_nbofsections, 1);

    if( (int)buf_nbofsections > 10 || (int)buf_nbofsections < 4)
    {
        printf("ERROR\nwrong sect_nr");
        return;
    }

    for(i=1; i <= (int)buf_nbofsections; i++)
    {
        read(fd, buf_sectname, 10);
        read(fd, &buf_secttype, 2);
        if(!(buf_secttype==32 || buf_secttype==54 || buf_secttype==14 || buf_secttype==76 || buf_secttype==18 || buf_secttype==83 ))
        {
            printf("ERROR\nwrong sect_types");
            return;
        }
        read(fd, &buf_sectoffset, 4);
        read(fd, &buf_sectsize,4);
        strcpy(buf_sectnames[i], buf_sectname);
        buf_secttypes[i] = buf_secttype;
        buf_sectsizes[i] = buf_sectsize;
    }

    printf("SUCCESS\nversion=%d\nnr_sections=%d\n", (int)buf_version, (int)buf_nbofsections);

    for(i=1; i <= (int)buf_nbofsections; i++)
    {
        printf("section%d: %s %d %ld\n", i, buf_sectnames[i], buf_secttypes[i], buf_sectsizes[i]);
    }

}

/**
* function for checking if a file is a section file with
* at least one section with section type 18
**/
int parseFindall(char *fileName)
{

    int fd;
    char buf_magic[4];
    short buf_size;
    char buf_version;
    char buf_nbofsections;
    char buf_sectname[10];
    short buf_secttype;
    long buf_sectoffset;
    long buf_sectsize;
    int i;
    int found = 0;

    if( (fd = open(fileName, O_RDONLY)) < 0)
    {
        return 0;
    }

    lseek(fd, -4, SEEK_END);
    read(fd, buf_magic, 4);

    if (strcmp(buf_magic, "mJcq") != 0 )
    {
        return 0 ;
    }

    lseek(fd, -6, SEEK_END);
    read(fd, &buf_size,2);
    lseek(fd, -buf_size, SEEK_END);
    read(fd, &buf_version, 1);

    if( (int)buf_version < 77 || (int)buf_version > 110)
    {
        return 0;
    }

    read(fd, &buf_nbofsections, 1);

    if( (int)buf_nbofsections > 10 || (int)buf_nbofsections < 4)
    {
        return 0 ;
    }

    for(i=1; i <= (int)buf_nbofsections; i++)
    {
        read(fd, buf_sectname, 10);
        read(fd, &buf_secttype, 2);
        if(!(buf_secttype==32 || buf_secttype==54 || buf_secttype==14 || buf_secttype==76 || buf_secttype==18 || buf_secttype==83 ))
        {
            return 0 ;
        }
        if ( buf_secttype == 18 )
        {
            found = 1;
        }
        read(fd, &buf_sectoffset, 4);
        read(fd, &buf_sectsize,4);
    }

    return found;
}

/**
* function for listing all the SF files with at least one
* section with section type 18
**/
void findall(char *dirName, int first)
{

    DIR *dir;
    struct dirent *dirEntry;
    char path[2048];

    if (!(dir = opendir(dirName)))
    {
        printf("ERROR\ninvalid directory path");
        return;
    }

    while ((dirEntry = readdir(dir)) != NULL)
    {
        snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
        if (first == 1)
        {
            printf("SUCCESS\n");
            first = 0;
        }
        if (dirEntry->d_type == DT_DIR)
        {
            if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
                continue;
            findall(path, 0);
        }
        else
        {
            snprintf(path, sizeof(path), "%s/%s", dirName, dirEntry->d_name);
            if ( parseFindall(path) == 1)
            {
                printf("%s\n", path);
            }
        }
    }

    closedir(dir);
}

/**
* function to check if a file is section file
**/
int isSectFile(char *fileName)
{

    int fd;
    char buf_magic[4];
    short buf_size;
    char buf_version;
    char buf_nbofsections;
    char buf_sectname[10];
    short buf_secttype;
    long buf_sectoffset;
    long buf_sectsize;
    int i;

    if( (fd = open(fileName, O_RDONLY)) < 0)
    {
        return 0;
    }

    lseek(fd, -4, SEEK_END);
    read(fd, buf_magic, 4);

    if (strcmp(buf_magic, "mJcq") != 0 )
    {
        return 0 ;
    }

    lseek(fd, -6, SEEK_END);
    read(fd, &buf_size,2);
    lseek(fd, -buf_size, SEEK_END);
    read(fd, &buf_version, 1);

    if( (int)buf_version < 77 || (int)buf_version > 110 )
    {
        return 0;
    }

    read(fd, &buf_nbofsections, 1);

    if( (int)buf_nbofsections > 10 || (int)buf_nbofsections < 4 )
    {
        return 0 ;
    }

    for(i=1; i <= (int)buf_nbofsections; i++)
    {
        read(fd, buf_sectname, 10);
        read(fd, &buf_secttype, 2);
        if(!(buf_secttype==32 || buf_secttype==54 || buf_secttype==14 || buf_secttype==76 || buf_secttype==18 || buf_secttype==83 ))
        {
            return 0 ;
        }
        read(fd, &buf_sectoffset, 4);
        read(fd, &buf_sectsize,4);
    }

    return 1;
}

/**
* function to extract a line from a section
**/
void extract(char *fileName, int section, int line)
{

    char buf_magic[4];
    short buf_size;
    char buf_version;
    char buf_nbofsections;
    char buf_sectname[10];
    short buf_secttype;
    long buf_sectoffset;
    long buf_sectsize;
    char section_text[2048];
    char line_text[1024];
    int i;
    int fd;
    int nbOfLines = 1;
    int found = 0;
    int pos = 0;

    if( (fd = open(fileName, O_RDONLY)) < 0)
    {
        printf("ERROR\ninvalid file");
        return;
    }

    if ( isSectFile(fileName) == 0)
    {
        printf("ERROR\ninvalid file");
        return;
    }

    lseek(fd, -4, SEEK_END);
    read(fd, buf_magic, 4);
    lseek(fd, -6, SEEK_END);
    read(fd, &buf_size,2);
    lseek(fd, -buf_size, SEEK_END);
    read(fd, &buf_version, 1);
    read(fd, &buf_nbofsections, 1);

    if( (int)buf_nbofsections < section )
    {
        printf("ERROR\ninvalid section");
        return;
    }

    for(i=1; i <= section; i++)
    {
        read(fd, buf_sectname, 10);
        read(fd, &buf_secttype, 2);
        read(fd, &buf_sectoffset, 4);
        read(fd, &buf_sectsize,4);
    }

    lseek(fd, buf_sectoffset, SEEK_SET);
    read(fd, &section_text, buf_sectsize);

    int j = 0;

    if ( line == 1)
    {
        for(i = strlen(section_text)-1; i >= 1; i--)
        {
            if( section_text[i] == 10 && section_text[i-1] == 13)
                break;
            if(isalnum(section_text[i]) || ispunct(section_text[i] || isspace(section_text[i])) )
            {
                line_text[j] = section_text[i];
                j++;
            }
        }

        printf("\nSUCCESS\n%s", line_text);
        return;
    }

    for(i = strlen(section_text)-1; i >= 1 && found == 0; i--)
    {
        if( section_text[i] == 10 && section_text[i-1] == 13)
            nbOfLines ++;
        if ( nbOfLines == line)
        {
            found = 1;
            pos = i-1;
            break;
        }
    }

    if ( nbOfLines < line )
    {
        printf("nb of lines: %d", nbOfLines);
        printf("ERROR\ninvalid line");
        return;
    }

    j = 0;
    int endLine = 0;

    for(i = pos-1; i >= 1 && endLine == 0; i--)
    {
        if( section_text[i] == 10 && section_text[i-1] == 13)
            endLine = 1;
        else
        {
            line_text[j] = section_text[i];
            j++;
        }
    }

    printf("SUCCESS\n%s", line_text);

}

/**
* function for checking if the input command is
*   list path=<dir_path>
**/
int checkSimpleListParam(char **argv)
{

    char pathbuff1[6];
    char pathbuff2[100];
    struct stat fileMetadata;
    memcpy(pathbuff1, &argv[2][0], 5);
    pathbuff1[5] = '\0';

    if( strcmp(pathbuff1,"path=") == 0)
    {
        memcpy(pathbuff2, &argv[2][5], strlen(argv[2]) - 5 );
        pathbuff2[strlen(argv[2]) - 5] = '\0';
        stat(pathbuff2, &fileMetadata);
        if (S_ISDIR(fileMetadata.st_mode))
        {
            listDir(pathbuff2);
        }
        return 1;
    }

    return 0;
}

/**
* function for checking if the input command is one of the following:
*   list recursive path=<dir_path>
*   list recursive has_perm_write path=<dir_path>
*   list recursive size_smaller=valut path=<dir_path>
**/
int checkRecParam(char **argv)
{
    char pathbuff1[6];
    char pathbuff2[100];
    char ssbuff1[14];
    char ssbuff2[10];
    int size;
    struct stat fileMetadata;

    if( strcmp(argv[2], "recursive") == 0)
    {
        memcpy(pathbuff1, &argv[3][0], 5);
        pathbuff1[5] = '\0';
        if( strcmp(pathbuff1,"path=") == 0)
        {
            memcpy(pathbuff2, &argv[3][5], strlen(argv[3]) - 5 );
            pathbuff2[strlen(argv[3]) - 5] = '\0';
            stat(pathbuff2, &fileMetadata);
            if (S_ISDIR(fileMetadata.st_mode))
            {
                listDirRec(pathbuff2,1);
            }

        }

        else if( strcmp(argv[3], "has_perm_write") == 0)
        {
            memcpy(pathbuff2, &argv[4][5], strlen(argv[4]) - 5 );
            pathbuff2[strlen(argv[4]) - 5] = '\0';
            stat(pathbuff2, &fileMetadata);
            if (S_ISDIR(fileMetadata.st_mode))
            {
                listPermWriteRec(pathbuff2,1);
            }
        }

        else
        {
            memcpy(ssbuff1, &argv[3][0], 13);
            ssbuff1[13] = '\0';
            if( strcmp(ssbuff1,"size_smaller=") == 0)
            {
                memcpy(ssbuff2, &argv[3][13], strlen(argv[3]) - 13 );
                ssbuff2[strlen(argv[3]) - 5] = '\0';
                size = atoi(ssbuff2);
                memcpy(pathbuff1, &argv[4][0], 5);
                pathbuff1[5] = '\0';
                if( strcmp(pathbuff1,"path=") == 0)
                {
                    memcpy(pathbuff2, &argv[4][5], strlen(argv[4]) - 5 );
                    pathbuff2[strlen(argv[4]) - 5] = '\0';
                    stat(pathbuff2, &fileMetadata);
                    if (S_ISDIR(fileMetadata.st_mode))
                    {
                        listSizeSmallerRec(pathbuff2, size, 1);
                    }
                }
            }
        }
        return 1;
    }
    return 0;
}

/**
* function for checking if the input command is one of the following:
*   list has_perm_write path=<dir_path>
*   list hs_perm_write recursive path=<dir_path>
**/
int checkPermWriteParam(char **argv)
{
    char pathbuff1[6];
    char pathbuff2[100];
    struct stat fileMetadata;

    if( strcmp(argv[2], "has_perm_write") == 0)
    {
        if( strcmp(argv[3], "recursive") == 0)
        {
            memcpy(pathbuff2, &argv[4][5], strlen(argv[4]) - 5 );
            pathbuff2[strlen(argv[4]) - 5] = '\0';
            stat(pathbuff2, &fileMetadata);
            if (S_ISDIR(fileMetadata.st_mode))
            {
                listPermWriteRec(pathbuff2,1);
            }
        }

        else
        {
            memcpy(pathbuff1, &argv[3][0], 5);
            pathbuff1[5] = '\0';
            if( strcmp(pathbuff1,"path=") == 0)
            {
                memcpy(pathbuff2, &argv[3][5], strlen(argv[3]) - 5 );
                pathbuff2[strlen(argv[3]) - 5] = '\0';
                stat(pathbuff2, &fileMetadata);
                if (S_ISDIR(fileMetadata.st_mode))
                {
                    listPermWrite(pathbuff2);
                }
            }
        }
        return 1;
    }
    return 0;
}

/**
* function for checking if the input command is
*   parse path=<file_path>
**/
int checkParseParam(char **argv)
{
    char pathbuff1[6];
    char pathbuff2[100];
    if(strcmp(argv[1], "parse") == 0)
    {
        memcpy(pathbuff1, &argv[2][0], 5);
        pathbuff1[5] = '\0';
        if( strcmp(pathbuff1,"path=") == 0)
        {
            memcpy(pathbuff2, &argv[2][5], strlen(argv[2]) - 5 );
            pathbuff2[strlen(argv[2]) - 5] = '\0';
            parse(pathbuff2);
        }
        return 1;
    }
    return 0;
}

/**
* function for checking if the input command is
*   findall path=<dir_path>
**/
int checkFindallParam(char **argv)
{
    char pathbuff1[6];
    char pathbuff2[100];
    struct stat fileMetadata;
    if(strcmp(argv[1], "findall") == 0 )
    {
        memcpy(pathbuff1, &argv[2][0], 5);
        pathbuff1[5] = '\0';
        if( strcmp(pathbuff1,"path=") == 0)
        {
            memcpy(pathbuff2, &argv[2][5], strlen(argv[2]) - 5 );
            pathbuff2[strlen(argv[2]) - 5] = '\0';
            stat(pathbuff2, &fileMetadata);
            if (S_ISDIR(fileMetadata.st_mode))
            {
                findall(pathbuff2, 1);
            }
        }

        return 1;
    }
    return 0;
}

/**
* function for checking if the input command is
*   extract path=<file_path> section=<sect_nr> line=<line_nr>
**/
int checkExtractParam(char **argv)
{
    char pathbuff1[6];
    char pathbuff2[100];
    char sectbuff1[9];
    char sectbuff2[10];
    char linebuff1[5];
    char linebuff2[10];
    int section;
    int line;

    if(strcmp(argv[1], "extract") == 0)
    {
        memcpy(pathbuff1, &argv[2][0], 5);
        pathbuff1[5] = '\0';
        if( strcmp(pathbuff1,"path=") == 0)
        {
            memcpy(pathbuff2, &argv[2][5], strlen(argv[2]) - 5 );
            pathbuff2[strlen(argv[2]) - 5] = '\0';
            memcpy(sectbuff1, &argv[3][0], 8);
            sectbuff1[9] = '\0';
            if( strcmp(sectbuff1, "section=") == 0 )
            {
                memcpy(sectbuff2, &argv[3][8], strlen(argv[3]) - 8);
                sectbuff2[strlen(argv[3]) - 8] = '\0';
                memcpy(linebuff1, &argv[4][0], 5);
                linebuff1[5] = '\0';
                if(strcmp(linebuff1, "line=") == 0)
                {
                    memcpy(linebuff2, &argv[4][5], strlen(argv[4]) - 5 );
                    linebuff2[strlen(argv[4]) - 5] = '\0';
                    section = atoi(sectbuff2);
                    line = atoi(linebuff2);
                    extract(pathbuff2, section, line);
                    return 1;
                }
                else return 0;
            }
            else return 0;
        }
        else return 0;
    }
    else return 0;
}

int main(int argc, char **argv)
{

    char pathbuff1[6];
    char pathbuff2[100];
    char ssbuff1[14];
    char ssbuff2[10];
    int size;
    struct stat fileMetadata;

    if(argc >= 2)
    {
        // check if first argument is "variant"
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("27829\n");
        }
        // check if first argument is "list"
        else if(strcmp(argv[1], "list") == 0)
        {
            if ( checkRecParam(argv) == 0 )
                if (checkPermWriteParam(argv) == 0)
                    if(checkSimpleListParam(argv) == 0)
                    {
                        memcpy(ssbuff1, &argv[2][0], 13);
                        ssbuff1[13] = '\0';
                        if( strcmp(ssbuff1,"size_smaller=") == 0)
                        {
                            memcpy(ssbuff2, &argv[2][13], strlen(argv[2]) - 13 );
                            ssbuff2[strlen(argv[2]) - 5] = '\0';
                            size = atoi(ssbuff2);
                            if( strcmp(argv[3], "recursive") == 0)
                            {
                                memcpy(pathbuff1, &argv[4][0], 5);
                                pathbuff1[5] = '\0';
                                if( strcmp(pathbuff1,"path=") == 0)
                                {
                                    memcpy(pathbuff2, &argv[4][5], strlen(argv[4]) - 5 );
                                    pathbuff2[strlen(argv[4]) - 5] = '\0';
                                    stat(pathbuff2, &fileMetadata);
                                    if (S_ISDIR(fileMetadata.st_mode))
                                    {
                                        listSizeSmallerRec(pathbuff2, size, 1);
                                    }
                                }
                            }

                            else
                            {
                                memcpy(pathbuff1, &argv[3][0], 5);
                                pathbuff1[5] = '\0';
                                if( strcmp(pathbuff1,"path=") == 0)
                                {
                                    memcpy(pathbuff2, &argv[3][5], strlen(argv[3]) - 5 );
                                    pathbuff2[strlen(argv[3]) - 5] = '\0';
                                    stat(pathbuff2, &fileMetadata);
                                    if (S_ISDIR(fileMetadata.st_mode))
                                    {
                                        listSizeSmaller(pathbuff2, size);
                                    }
                                }
                            }
                        }
                    }

        }

        else if (checkExtractParam(argv) == 0 )
        {
            if ( checkParseParam(argv) == 0)
            {
                checkFindallParam(argv);
            }
        }
    }
    return 0;
}
