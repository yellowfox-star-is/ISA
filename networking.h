#ifndef NETWORKING_H
#define NETWORKING_H

void *get_addr(struct sockaddr *sa);
int send_data(int socket, const struct addrinfo *serverinfo,unsigned char *data, int data_length);
int get_address_info(struct addrinfo *hints, struct addrinfo **serverinfo, char *hostname);
int initialize_socket(struct addrinfo *serverinfo);
//extern 

#endif //NETWORKING_H