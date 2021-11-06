#ifndef NETWORKING_H
#define NETWORKING_H

void *get_addr(struct sockaddr *sa);
int send_data(int socket, const struct addrinfo *serverinfo, const char *fmt, ...);
int get_address_info(struct addrinfo *hints, struct addrinfo **serverinfo, char *hostname);
int initialize_socket(struct addrinfo *serverinfo);

#endif //NETWORKING_H