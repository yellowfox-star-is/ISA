//author: Václav Sysel
//VUT FIT, 5. semestr
//ISA, projekt
//varianta: skrytý kanál

//POSIX headers
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//Network libraries
#include <netdb.h>
#include <arpa/inet.h>

//custom headers
#include "error.h"
#include "bit.h"

int read_arguments(int argc, char* argv[], char **filename, char **hostname, bool *isServer, bool *isVerbose);
int verify_arguments(char *filename, char *hostname, bool isServer, bool isVerbose);

void *get_addr(struct sockaddr *sa)
{
    if (sa == NULL)
    {
        return NULL;
    }

    if (sa->sa_family == AF_INET) //is ipv4
    {
        return &(((struct sockaddr_in *) sa ) -> sin_addr);
    }
    else if (sa->sa_family == AF_INET6) //is ipv6
    {
        return &(((struct sockaddr_in6 *) sa ) -> sin6_addr);
    }

    return NULL;
}

int main (int argc, char* argv[])
{
    char *filename = NULL;
    char *hostname = NULL;
    bool isServer = false;
    bool isVerbose = false;

    read_arguments(argc, argv, &filename, &hostname, &isServer, &isVerbose);
    verify_arguments(filename, hostname, isServer, isVerbose);
    

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
        printf("Recieved arguments:\n");
        printf("    filename: %s\n", filename);
        printf("    hostname: %s\n", hostname);
        printf("    isServer: %s\n", BOOL2STRING(isServer));
        printf("    isVerbose: %s\n", BOOL2STRING(isVerbose));
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
                if (BIT_GET(state, 0)) printf("    filename: %s\n", filename);
                if (BIT_GET(state, 1)) printf("    ip|hostname: %s\n", hostname);
                printf("These arguments won't be used and checked, as they are not needed for server mode.\n");
            }
        }
        return 0;
    }
    else //isClient
    {
        if (access(filename, F_OK)) //checks if file exists
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
            if (access(filename, R_OK)) //checks if it is readable
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
        struct addrinfo hints; //prepares structs for function getaddrinfo
        struct addrinfo *serverinfo;
        //replaces all bytes in struct hints with zeroes
        //it is from strings.h but it works nicely in this situation
        memset(&hints, 0, sizeof(hints)); 
        int getadd_result = 0;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_RAW;

        getadd_result = getaddrinfo(hostname, NULL, &hints, &serverinfo);

        if (getadd_result != 0)
        {
            warning_msg("Couldn't verify hostname: %s\n    %s\n",hostname, gai_strerror(getadd_result));
            result = 1;
        }
        else if (isVerbose)
        {
            printf("Verified that hostname \"%s\" can be used.\n", hostname);
            char ip[100];
            inet_ntop(serverinfo->ai_family, get_addr(serverinfo->ai_addr), ip, 100);
            printf("Translated hostname to: %s\n", ip);
        }

    }

    if (result)
    {
        error_exit(result, "Program wasn't able to use passed arguments.\n");
    }

    return result;
}
