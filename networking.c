#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h> //TODO CRITICAL: find what the fuck is this and find where I could have stolen prepare_icmp_header!!!!!
#include <sys/socket.h>
#include <openssl/aes.h>

#include "error.h"
#include "networking.h"
#include "smrcka_bat.h"

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

int initialize_socket(struct addrinfo *serverinfo)
{
    int protocol = serverinfo->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;
	return socket(serverinfo->ai_family, serverinfo->ai_socktype, protocol);
}

int get_address_info(struct addrinfo *hints, struct addrinfo **serverinfo, char *hostname)
{
    //replaces all bytes in struct hints with zeroes
    //it is from strings.h but it works nicely in this situation
    memset(hints, 0, sizeof(*hints));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_RAW;
    return getaddrinfo(hostname, NULL, hints, serverinfo);
}

int send_data(int socket, const struct addrinfo *serverinfo, const char *fmt, ...)
{
    //variables initialization
    char packet[MAX_PACKET_LENGTH];
    char data[MAX_DATA_LENGTH];
    char encrypted_data[MAX_ENCRYPTED_DATA_LENGTH];
    int data_length = 0;
    int encrypted_data_length = 0;
    

    //data preparation
    va_list args;
    va_start(args, fmt);
    data_length = snprintf(data, MAX_DATA_LENGTH, fmt, args);
    va_end(args);

    if (data_length < 0 || data_length >= MAX_DATA_LENGTH)
    {
        warning_msg("Implementation error. Failed during packet preparation. Data too large.\n");
        warning_msg("Please contact author.\n");
        return 1;
    }

    //TODO ADD ENCRYPTION
    //TODO USE EVP?
    //encryption
    AES_KEY key;
    AES_set_encrypt_key((const unsigned char *)"xsysel09", 128, &key);
    //TODO continue
    if (encrypted_data_length >= MAX_ENCRYPTED_DATA_LENGTH)
    {
        warning_msg("Implementation error. Failed during packet preparation. Encrypted data too large.\n");
        warning_msg("Please contact author.\n");
        return 1;
    }

    //packet preparation
    memset(&packet, 0, 1500);
    struct icmphdr *icmp_header = (struct icmphdr *)packet;
    icmp_header->code = ICMP_ECHO;
    icmp_header->checksum = 0;
    //TODO ADD CHECKSUM CALCULATION

    memcpy(packet + sizeof(struct icmphdr), data, encrypted_data_length);

    if (sendto(socket, packet, sizeof(struct icmphdr) + encrypted_data_length, 0, (struct sockaddr *)(serverinfo->ai_addr), serverinfo->ai_addrlen) < 0)
    {
        warning_msg("Program wasn't able to send packet.\n");
        return 1;
    }

    return 0;
}