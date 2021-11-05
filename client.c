#include <stdbool.h>

enum client_state {START, SEND_HEADER, WAIT_FOR_HEADER, SEND_DATA, WAIT_FOR_ACCEPT, SEND_LAST_PACKET, WAIT_FOR_FINISH, END, EXIT};

int start_client(char *filename, char *hostname, bool isVerbose)
{

    enum client_state state = START;

    switch (state)
    {
        case START:
            initialize_sender();
            state = SEND_HEADER;
        break;

        case SEND_HEADER:
            send_data();
            state = WAIT_FOR_ACCEPT;
        break;
    }
}