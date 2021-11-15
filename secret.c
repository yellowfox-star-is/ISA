//author: Václav Sysel
//VUT FIT, 5. semestr
//ISA, projekt
//varianta: skrytý kanál

//POSIX headers
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

//custom headers
#include "error.h"
#include "bit.h"
#include "arguments.h"
#include "smrcka_bat.h"

#include "client.h"
#include "server.h"

void intHandler(int dummy)
{
    (void)dummy;
    fprintf(ERR_OUT, "\n");
    warning_msg("Interrupt received. Program will close shortly.\n");
    cancel_received = true;
}

int main (int argc, char* argv[])
{
    char *filename = NULL;
    char *hostname = NULL;
    bool isServer = false;
    bool isVerbose = false;

    read_arguments(argc, argv, &filename, &hostname, &isServer, &isVerbose);
    verify_arguments(argv, filename, hostname, isServer, isVerbose);

    signal(SIGINT, intHandler);

    if (isServer)
    {
        start_server(isVerbose);
    }
    else
    {
        start_client(filename, hostname, isVerbose);
    }

    return 0;
}