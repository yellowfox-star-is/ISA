#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "networking.h"

void *get_addr(struct sockaddr *sa)
{
    if (sa == NULL)
    {
        return NULL;
    }

    if (sa->sa_family == AF_INET) //is ipv4
    {
        return &(((struct sockaddr_in *) sa ) -> sin_addr);
    }
    else if (sa->sa_family == AF_INET6) //is ipv6
    {
        return &(((struct sockaddr_in6 *) sa ) -> sin6_addr);
    }

    return NULL;
}