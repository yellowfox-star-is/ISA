#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "error.h"
#include "smrcka_bat.h"
#include "networking.h"

enum server_state {WAIT_FOR_HEADER, REPEAT_HEADER, ACCEPT_HEADER, WAIT_FOR_DATA, REPEAT_DATA, ACCEPT_DATA, ACCEPT_END, FINISH, CANCEL, EXIT};

int copy_something(char *target, char *source, int max_length)
{
    char *ptr = NULL;
    if ((ptr = strchr(source, '\n')) != NULL)
    {
        int i;
        ptr += 1;
        for (i = 0 ; i < max_length && source[i] != '\n' && source[i] != '\0' ; i++)
        {
            target[i] = source[i];
        }
        for (;i < max_length; i++)
        {
            target[i] = '\0';
        }

        return 0;
    }
    return 1;
}

int copy_filename(char *target, char *source, int max_length)
{
    if (strcmp(source, "SECRET_START\n"))
    {
        return copy_something(target, source, max_length);
    }
    return 1;
}

int copy_length(char *source)
{
    char tmp[CHAR_LIMIT];
    if (strcmp(source, "SECRET_DATA\n") || strcmp(source, "SECRET_END\n"))
    {
        if (copy_something(tmp, source, CHAR_LIMIT))
        {
            return atoi(tmp);
        }
    }
    return -1;
}

int change_filename(char *filename)
{
    char new_filename[CHAR_LIMIT];
    char new_end[CHAR_LIMIT];
    for (unsigned int i = 0; i < UINT_MAX; i++)
    {
        strncpy(new_filename, filename, CHAR_LIMIT);
        snprintf(new_end, CHAR_LIMIT, ".%d", i);
        strncat(new_filename, new_end, CHAR_LIMIT);
        if (!access(new_filename, F_OK))
        {
            strcpy(filename, new_filename);
            return 0;
        }
    }
    warning_msg("An alternative filename couldn't be found.\n");
    return 1;
}

int write_to_file(FILE *file, char *stream, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (fprintf(file, "%c", stream[i]) <= 0)
        {
            warning_msg("Failed writing to file.\n");
            return -1;
        }
    }
    return 0;
}

//taken from: https://stackoverflow.com/questions/46070363/how-to-use-strchr-multiple-times-to-find-the-nth-occurrence/46070740
const char* nth_strchr(const char* s, int c, int n)
{
    int c_count;
    char* nth_ptr;

    for (c_count=1,nth_ptr=strchr(s,c); 
        nth_ptr != NULL && c_count < n && c!=0; 
        c_count++)
    {
        nth_ptr = strchr(nth_ptr+1, c);
    }

    return nth_ptr;
}

int start_server(bool isVerbose)
{
    //server state
    int result = 0;
    enum server_state state = WAIT_FOR_HEADER;

    //file info
    FILE *file = NULL;
    char filename[CHAR_LIMIT];
    filename[0] = '\0';

    //client info
    char clientname[CHAR_LIMIT];
    clientname[0] = '\0';

    //networking
    int socket = 0;
    struct addrinfo hints;
    struct addrinfo *serverinfo;

    int out_data_length = 0;
    unsigned char out_data[MAX_DATA_LENGTH];

    int new_in_data_length = 0;
    int in_data_length = 0;
    char *in_data;

    while (state != EXIT)
    {
        switch (state)
        {
            case WAIT_FOR_HEADER:
                listen_for_packet(isVerbose);
                if (packet_was_caught)
                {
                    if (recognized_protocol == SECRET_CORRUPTED)
                    {
                        //TODO CRITICAL WARNING, the socket can be not initialized yet
                        state = REPEAT_HEADER;
                        break;
                    }
                    if (recognized_protocol == SECRET_START)
                    {
                        //initialization
                        strcpy(clientname, ip_src);
                        if (get_address_info(&hints, &serverinfo, clientname))
                        {
                            error_exit(1, "Couldn't resolve clients name.\n");
                        }
                        socket = initialize_socket(serverinfo);
                        if (socket == -1)
                        {
                            error_exit(1, "Program wasn't able to open a socket.\n");
                        }
                        if (copy_filename(filename, decrypted_packet, CHAR_LIMIT))
                        {
                            error_exit(1, "Filename is bigger than limit of program.\n");
                        }
                        if (access(filename, F_OK)) //If filename already exists
                        {
                            warning_msg("File name already exists.\n");
                            if (replace_file)
                            {
                                warning_msg("Original file will be replaced.\n");
                            }
                            else
                            {
                                warning_msg("Another name will be used.\n");
                                if (change_filename(filename))
                                {
                                    exit(1);
                                }
                                warning_msg("File will be saved as: %s\n", filename);
                            }
                        }
                        file = fopen(filename, "w");
                        if (file == NULL)
                        {
                            error_exit(1, "Program wasn't able to open a file.\n");
                        }
                        state = ACCEPT_HEADER;
                        break;
                    }
                    if (isVerbose)
                    {
                        printf("Unexcpected secret packet recieved. Was waiting for header.\n");
                    }
                }
            break;

            case REPEAT_HEADER:
                out_data_length = snprintf((char *)out_data, MAX_DATA_LENGTH, "SECRET_REPEAT\n");
                send_data(socket, serverinfo, out_data, out_data_length);
                state = WAIT_FOR_HEADER;
                break;
            break;

            case ACCEPT_HEADER:
                out_data_length = snprintf((char *)out_data, MAX_DATA_LENGTH, "SECRET_ACCEPT\n");
                send_data(socket, serverinfo, out_data, out_data_length);
                state = WAIT_FOR_DATA;
                break;
            break;

            case WAIT_FOR_DATA:
                listen_for_packet(isVerbose);
                if (packet_was_caught && !strcmp(clientname, ip_src))
                {
                    if (recognized_protocol == SECRET_CORRUPTED)
                    {
                        state = REPEAT_DATA;
                        break;
                    }
                    if (recognized_protocol == SECRET_START)
                    {
                        //we will presume that:
                        //    the filename is same
                        //    the client didn't get accept
                        //    and that is why they are sending header again
                        //    program doesn't check if that is actually truth
                        //    in theory client could have started another file transaction
                        state = ACCEPT_HEADER;
                        break;
                    }
                    if (recognized_protocol == SECRET_DATA || recognized_protocol == SECRET_END)
                    {
                        new_in_data_length = copy_length(decrypted_packet);
                        if (new_in_data_length == 0)
                        {
                            state = REPEAT_DATA;
                            break;
                        }
                        if (new_in_data_length == in_data_length)
                        {
                            //we cannot tell now, if the packets are same or not
                            //one way would be to implement timestamps, but because of time press
                            //they aren't implemented
                        }
                        in_data_length = new_in_data_length;
                        if ((in_data = (char *)(nth_strchr(decrypted_packet, '\n', 2) + 1)) == NULL)
                        {
                            if (isVerbose)
                            {
                                warning_msg("Recieved unvalid data.\n");
                                state = REPEAT_DATA;
                                break;
                            }
                        }
                        if (write_to_file(file, in_data, in_data_length))
                        {
                            state = CANCEL;
                            break;
                        }
                        state = recognized_protocol == SECRET_DATA ? ACCEPT_DATA : ACCEPT_END;
                        break;
                    }
                }
            break;

            case REPEAT_DATA:
                out_data_length = snprintf((char *)out_data, MAX_DATA_LENGTH, "SECRET_REPEAT\n");
                send_data(socket, serverinfo, out_data, out_data_length);
                state = WAIT_FOR_DATA;
                break;
            break;

            case ACCEPT_DATA:
            case ACCEPT_END:
                out_data_length = snprintf((char *)out_data, MAX_DATA_LENGTH, "SECRET_ACCEPT\n");
                send_data(socket, serverinfo, out_data, out_data_length);
                state = state == ACCEPT_DATA ? WAIT_FOR_DATA : FINISH;
            break;

            case FINISH:
                fclose(file);
                state = EXIT;
                break;
            break;

            case EXIT:
                warning_msg("Implementation error. Server got into unrecognized state.\n");
                warning_msg("Please contact author.\n");
                result = 1;
                state = EXIT;
            break;

            case CANCEL:
                result = 1;
                if (file != NULL)
                {
                    fclose(file);
                }
                if (filename[0] != '\0')
                {
                    remove(filename);
                }
                state = EXIT;
            break;
        }
    }
    return result;
}