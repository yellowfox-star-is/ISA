#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"
#include "smrcka_bat.h"
#include "networking.h"

enum server_state {WAIT_FOR_HEADER, CANCEL, EXIT};

int start_server(bool isVerbose)
{
    int result = 0;
    enum server_state state = WAIT_FOR_HEADER;
    FILE *file = NULL;
    unsigned char buffer[MAX_PACKET_LENGTH];
    char filename[CHAR_LIMIT];
    char clientname[CHAR_LIMIT];
    filename[0] = '\0';
    clientname[0] = '\0';

    while (state != EXIT)
    {
        switch (state)
        {
            case WAIT_FOR_HEADER:

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
}