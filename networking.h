#ifndef NETWORKING_H
#define NETWORKING_H

#include "smrcka_bat.h"

void *get_addr(struct sockaddr *sa);
int send_data(int socket, const struct addrinfo *serverinfo,unsigned char *data, int data_length);
int get_address_info(struct addrinfo *hints, struct addrinfo **serverinfo, char *hostname);
int initialize_socket(struct addrinfo *serverinfo);
int listen_for_packet(bool isVerbose);

extern bool packet_was_caught;
extern enum secret_protocol recognized_protocol;
extern char ip_src[CHAR_LIMIT];
extern char decrypted_packet[MAX_PACKET_LENGTH]; 

#endif //NETWORKING_H