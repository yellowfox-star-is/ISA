#include <openssl/asn1.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include "error.h"
#include "smrcka_bat.h"
#include "client.h"
#include "networking.h"

enum client_state {START, SEND_HEADER, WAIT_FOR_HEADER, SEND_DATA, WAIT_FOR_ACCEPT, REPEAT_DATA, END, REPEAT_END, WAIT_FOR_FINISH, FREE, EXIT};

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

    while ((i < FILE_CHUNK) && (c = fgetc(file_input)) != EOF)
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
    int buffer_length = 0;
    struct addrinfo hints;
    struct addrinfo *serverinfo;
    int socket = 0;
    unsigned char buffer[MAX_DATA_LENGTH];
    unsigned char data[MAX_DATA_LENGTH];
    int data_length = 0;
    int soft_strikes = 0;
    int hard_strikes = 0;

    while (state != EXIT)
    {
        if (cancel_received)
        {
            state = FREE;
        }
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
                //TODO CRITICAL send only filename and not path
                data_length = snprintf((char *)data, MAX_DATA_LENGTH, "SECRET_START\n%s\n", filename);
                send_data(socket, serverinfo, data, data_length);
                state = WAIT_FOR_HEADER;
            break;

            case WAIT_FOR_HEADER:
            case WAIT_FOR_ACCEPT:
            case WAIT_FOR_FINISH:
                listen_for_packet(isVerbose);
                if (packet_was_caught) //maybe add IP check
                {
                    if (recognized_protocol == SECRET_REPEAT)
                    {
                        //if repeat is received, function will fall through and return to repeat state
                        soft_strikes--;
                    }
                    else if (recognized_protocol == SECRET_ACCEPT)
                    {
                        soft_strikes = 0;
                        hard_strikes = 0;
                        data_length = 0;
                        if (hard_strikes >= STRIKE_LIMIT)
                        {
                            warning_msg("Server resumed communication.\n");
                        }
                        state = state == WAIT_FOR_FINISH ? FREE : SEND_DATA;
                        break;
                    }
                }
                if (state == WAIT_FOR_HEADER) state = SEND_HEADER;
                if (state == WAIT_FOR_ACCEPT) state = REPEAT_DATA;
                if (state == WAIT_FOR_FINISH) state = REPEAT_END;
                soft_strikes++;
                hard_strikes++;
                if (hard_strikes == STRIKE_LIMIT)
                {
                    hard_strikes++;
                    if (soft_strikes == STRIKE_LIMIT)
                    {
                        warning_msg("Program is stuck on repeating messages.\n");
                    }
                    else
                    {
                        if (state == WAIT_FOR_FINISH)
                        {
                            warning_msg("Program didn't recieve confirmation of recieving of file, but all data were sent.\n");
                            warning_msg("Data were probably send and the program will close itself.\n");
                        }
                        else
                        {
                            warning_msg("Server stopped to communicate.\n");
                        }
                    }
                }
                else if (hard_strikes < STRIKE_LIMIT)
                {
                }
            break;

            case SEND_DATA:
                buffer_length = fill_buffer(file_input, buffer);
                if (buffer_length <= 0)
                {
                    buffer_length = abs(buffer_length);
                    state = END;
                    break;
                }
                data_length += snprintf((char *)data, MAX_DATA_LENGTH, "SECRET_DATA\n%d\n", buffer_length);
                memcpy(data + data_length, buffer, buffer_length);
                data_length += buffer_length;
                __attribute__ ((fallthrough));
            case REPEAT_DATA:
                send_data(socket, serverinfo, data, data_length);
                state = WAIT_FOR_ACCEPT;
            break;

/*
            case WAIT_FOR_ACCEPT:
                if (0)
                {
                    state = REPEAT_DATA;
                }
                else
                {
                    data_length = 0;
                    state = SEND_DATA;
                }
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
                    data_length = 0;
                }*/

            case END:
                data_length += snprintf((char *)data, MAX_DATA_LENGTH, "SECRET_END\n%d\n", buffer_length);
                memcpy(data + data_length, buffer, buffer_length);
                data_length += buffer_length;
                __attribute__ ((fallthrough));
            case REPEAT_END:
                send_data(socket, serverinfo, data, data_length);
                state = WAIT_FOR_FINISH;
            break;

            case FREE:
                fclose(file_input);
                state = EXIT;
            break;

            default:
                warning_msg("Implementation error. Client got into unrecognized state.\n");
                warning_msg("Please contact author.\n");
                result = 1;
                state = FREE;
            break;
        }
    }

    return result;
}
