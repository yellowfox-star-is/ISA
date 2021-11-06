#ifndef NETWORKING_H
#define NETWORKING_H

void *get_addr(struct sockaddr *sa);
int send_data(int socket, const struct addrinfo *serverinfo, const char *fmt, ...);

#endif //NETWORKING_H