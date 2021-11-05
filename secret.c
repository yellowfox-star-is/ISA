//author: Václav Sysel
//VUT FIT, 5. semestr
//ISA, projekt
//varianta: skrytý kanál

//POSIX headers
#include <stdlib.h>
#include <stdbool.h>

//custom headers
#include "error.h"
#include "bit.h"
#include "arguments.h"

#include "client.h"
//#include "server.h"

int main (int argc, char* argv[])
{
    char *filename = NULL;
    char *hostname = NULL;
    bool isServer = false;
    bool isVerbose = false;

    read_arguments(argc, argv, &filename, &hostname, &isServer, &isVerbose);
    verify_arguments(argv, filename, hostname, isServer, isVerbose);
    if (isServer)
    {
        //start_server(isVerbose);
    }
    else
    {
        //start_client(filename, hostname, isVerbose);
    }

    return 0;
}