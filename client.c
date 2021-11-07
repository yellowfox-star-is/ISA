#include <openssl/asn1.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#include "error.h"
#include "smrcka_bat.h"
#include "client.h"
#include "networking.h"

enum client_state {START, SEND_HEADER, WAIT_FOR_HEADER, SEND_DATA, WAIT_FOR_ACCEPT, REPEAT_DATA, END, WAIT_FOR_FINISH, EXIT};

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

int fill_buffer(FILE *file_input, unsigned char buffer[])
{
    int c;
    int i = 0;

    while ((i < MAX_DATA_LENGTH) && (c = fgetc(file_input)) != EOF)
    {
        buffer[i] = (char) c;
        i++;
    }

    //TODO WARNING maybe needed to put /0 on end
    return c == EOF ? i * -1 : i;
}

int start_client(char *filename, char *hostname, bool isVerbose)
{

    enum client_state state = START;
    FILE *file_input = NULL;
    int result = 0;
    int copied_length = 0;
    struct addrinfo hints;
    struct addrinfo *serverinfo;
    int socket = 0;
    unsigned char buffer[MAX_DATA_LENGTH];

    while (state != EXIT)
    {
        switch (state)
        {
            case START:
                result = get_address_info(&hints, &serverinfo, hostname);
                socket = initialize_socket(serverinfo);
                if (socket == -1)
                {
                    result = 1;
                    error_exit(1, "Program wasn't able to open a socket.\n");
                }
                initialize_file(filename, &file_input);
                if (file_input == NULL)
                {
                    result = 1;
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
                if (0)
                {
                    state = SEND_HEADER;
                }
                else
                {
                    state = SEND_DATA;
                }
            break;

            case SEND_DATA:
                copied_length = fill_buffer(file_input, buffer);
                __attribute__ ((fallthrough));
            case REPEAT_DATA:
                if (copied_length <= 0)
                {
                    state = END;
                    break;
                }
                send_data(socket, serverinfo, "SECRET\n%s", buffer);
                state = WAIT_FOR_ACCEPT;
            break;

            case WAIT_FOR_ACCEPT:
                if (0)
                {
                    state = REPEAT_DATA;
                }
                else
                {
                    state = SEND_DATA;
                }
            break;

            case END:
                send_data(socket, serverinfo, "SECRET\n%s\nSECRET_END\n", buffer);
                state = WAIT_FOR_FINISH;
            break;

            case WAIT_FOR_FINISH:
                //dummy wait
                if (0)
                {
                    state = END;
                }
                else
                {
                    state = EXIT;
                }
            break;

            case EXIT:
                fclose(file_input);
                state = EXIT;
            break;
        }
    }

    return result;
}
