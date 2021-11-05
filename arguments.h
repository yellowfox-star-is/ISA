#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdbool.h>

int read_arguments(int argc, char* argv[], char **filename, char **hostname, bool *isServer, bool *isVerbose);
int verify_arguments(char* argv[], char *filename, char *hostname, bool isServer, bool isVerbose);

#endif //ARGUMENTS_H