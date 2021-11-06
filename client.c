#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "smrcka_bat.h"
#include "client.h"
#include "networking.h"

enum client_state {START, SEND_HEADER, WAIT_FOR_HEADER, SEND_DATA, WAIT_FOR_ACCEPT, SEND_LAST_PACKET, WAIT_FOR_FINISH, END, EXIT};

int initialize_socket(char *hostname)
{
    //dummy socket
    return 0;
}

int close_socket()
{
    //dummy socket
    return 0;
}

int initialize_file(char *filename, FILE **file_input)
{
    if (filename == NULL)
    {
        *file_input = NULL;
        return 1;
    }

    *file_input = fopen(filename, "r");
    if (*file_input == NULL)
    {
        return 1;
    }
    
    return 0;
}

int start_client(char *filename, char *hostname, bool isVerbose)
{

    enum client_state state = START;
    FILE *file_input = NULL;
    int result = 0;
    int copied_length = 0;

    while (state != EXIT)
    {
        switch (state)
        {
            case START:
                result = initialize_socket(hostname);
                if (result)
                {
                    result = 1;
                    error_exit(1, "Program wasn't able to open a socket.\n");
                }
                initialize_file(filename, &file_input);
                if (file_input == NULL)
                {
                    result = 1;
                    close_socket();
                    error_exit(1, "Program wasn't able to open a file.\n");
                }
                state = SEND_HEADER;
            break;

            case SEND_HEADER:
                send_data(socket, serverinfo, "START_SECRET\n%s\n", hostname);
                state = WAIT_FOR_HEADER;
            break;

            case WAIT_FOR_HEADER:
                //dummy accept
                state = SEND_DATA;
            break;

            case SEND_DATA:
                //send_data();
                state = END;
                //state = WAIT_FOR_ACCEPT;
            break;

            case END:
                fclose(file_input);
                close_socket();
                state = EXIT;
            break;
        }
    }

    return result;
}