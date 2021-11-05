//author: Václav Sysel
//VUT FIT, 5. semestr
//ISA, projekt
//varianta: skrytý kanál

//POSIX headers
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Network libraries

//custom headers
#include "error.h"
#include "bit.h"
#include "arguments.h"

int main (int argc, char* argv[])
{
    char *filename = NULL;
    char *hostname = NULL;
    bool isServer = false;
    bool isVerbose = false;

    read_arguments(argc, argv, &filename, &hostname, &isServer, &isVerbose);
    verify_arguments(argv, filename, hostname, isServer, isVerbose);

    return 0;
}