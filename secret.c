//author: Václav Sysel
//VUT FIT, 5. semestr
//ISA, projekt
//varianta: skrytý kanál

//POSIX headers
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>

//Network libraries

//custom headers
#include "error.h"
#include "bit.h"

int read_arguments(int argc, char* argv[], char **filename, char **hostname, bool *isServer, bool *isVerbose);
int verify_arguments(char *filename, char *hostname, bool isServer, bool isVerbose);

int main (int argc, char* argv[])
{
    char *filename = NULL;
    char *hostname = NULL;
    bool isServer = false;
    bool isVerbose = false;

    read_arguments(argc, argv, &filename, &hostname, &isServer, &isVerbose);
    verify_arguments(filename, hostname, isServer, isVerbose);
    
    printf("Recieved:\n\
            filename: %s\n\
            hostname: %s\n\
            isServer: %d\n\
            isVerbose: %d\n",
            filename, hostname, isServer, isVerbose);

    return 0;
}

int read_arguments(int argc, char* argv[], char **filename, char **hostname, bool *isServer, bool *isVerbose)
{
    int original_opterr = opterr;
    opterr = 0;

    int result = 0;
    int c;

    while ((c = getopt(argc, argv, "r:s:lv")) != -1)
        switch (c)
        {
            case 'r':
                *filename = optarg;
                break;
            case 's':
                *hostname = optarg;
                break;
            case 'l':
                *isServer = true;
                break;
            case 'v':
                *isVerbose = true;
                break;

            case '?':
                switch (optopt)
                {
                    case 'r':
                        warning_msg("Option -r <file> requires an argument.\n");
                        break;
                    case 's':
                        warning_msg("Option -s <ip|hostname> requires an argument.\n");
                        break;
                    default:
                        warning_msg("Unknown option '-%c' will be skipped.\n", optopt);
                        break;
                }
                result = 1;
                break;
            default:
                error_exit(1, "Crittical error happened during argument parsing.\n");
                continue;
        }

    opterr = original_opterr;
    return result;
}

int verify_arguments(char *filename, char *hostname, bool isServer, bool isVerbose)
{
    int result = 0;

    if (isVerbose)
    {
        printf("The program was started in a verbose mode. Prepare, that it will be unnecesarily chatty.\n");
    }

    if (isServer)
    {
        if (isVerbose)
        {
            char state = 0;
            if (filename != NULL) BIT_SET(state, 0);
            if (hostname != NULL) BIT_SET(state, 1);
            if (state)
            {
                printf("Recieved unnecesary arguments:\n");
                if (BIT_GET(state, 0)) printf("filename: %s", filename);
                if (BIT_GET(state, 1)) printf("ip|hostname: %s", hostname);
                printf("Theese arguments won't be used and checked, as they are not needed for server mode.\n");
            }
        }
        return 0;
    }
    else //isClient
    {
       if (access(filename, F_OK))
       {
           warning_msg("File \"%s\" doesn't exists.\n", filename);
           result = 1;
       }
       else
       {
           if (isVerbose)
           {
               printf("File \"%s\" exists.\n", filename);
           }
           if (access(filename, R_OK))
           {
               warning_msg("File \"%s\" cannot be accessed for reading.\n", filename);
               result = 1;
           }
           else
           {
               if (isVerbose)
               {
                   printf("File \"%s\" can be accessed for reading.\n", filename);
               }
           }
       }

     //TODO continue, check if ip|address is legit
     
    }

    return result;
}
