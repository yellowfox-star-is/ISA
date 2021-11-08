#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h> //TODO CRITICAL: find what the fuck is this and find where I could have stolen prepare_icmp_header!!!!!
#include <sys/socket.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

//TODO Verify, is following can be used
#include <pcap.h>
#include <pcap/pcap.h>
#include <errno.h>
#define __FAVOR_BSD          // important for tcphdr structure
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> 
#include <err.h>

#ifdef __linux__            // for Linux
#include <netinet/ether.h> 
#include <time.h>
#include <pcap/pcap.h>
#endif
//TODO End

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

//handleErrors taken from https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int handleErrors(void)
{
    ERR_print_errors_fp(ERR_OUT);
    return EXIT_FAILURE;
}

//encrypt taken from https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        return handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key_128, NULL))
        return handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int send_data(int socket, const struct addrinfo *serverinfo,unsigned char *data, int data_length)
{
    //variables initialization
    unsigned char packet[MAX_PACKET_LENGTH];
    unsigned char encrypted_data[MAX_ENCRYPTED_DATA_LENGTH];
    int encrypted_data_length = 0;
    
    #if 0
    //data preparation
    va_list args;
    va_start(args, fmt);
    data_length = snprintf((char *)data, MAX_DATA_LENGTH, fmt, args);
    va_end(args);
    #endif

    if (data_length < 0 || data_length >= MAX_DATA_LENGTH)
    {
        warning_msg("Implementation error. Failed during packet preparation. Data too large.\n");
        warning_msg("Please contact author.\n");
        return 1;
    }

    //TODO ADD ENCRYPTION
    //TODO USE EVP?
    //encryption
    #if 0
    encrypted_data_length = encrypt(data, data_length, encrypted_data);
    //TODO continue
    if (encrypted_data_length >= MAX_ENCRYPTED_DATA_LENGTH)
    {
        warning_msg("Implementation error. Failed during packet preparation. Encrypted data too large.\n");
        warning_msg("Please contact author.\n");
        return 1;
    }
    #endif

    //packet preparation
    memset(&packet, 0, 1500);
    struct icmphdr *icmp_header = (struct icmphdr *)packet;
    icmp_header->code = ICMP_ECHO;
    icmp_header->checksum = 0;
    //TODO ADD CHECKSUM CALCULATION

    memcpy(packet + sizeof(struct icmphdr), data, data_length);

    if (sendto(socket, packet, sizeof(struct icmphdr) + data_length, 0, (struct sockaddr *)(serverinfo->ai_addr), serverinfo->ai_addrlen) < 0)
    {
        warning_msg("Program wasn't able to send packet.\n");
        return 1;
    }

    return 0;
}

void mypcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
  struct ip *my_ip;               // pointer to the beginning of IP header
  struct ether_header *eptr;      // pointer to the beginning of Ethernet header
  const struct tcphdr *my_tcp;    // pointer to the beginning of TCP header
  const struct udphdr *my_udp;    // pointer to the beginning of UDP header
  u_int size_ip;
}

/*==========================================================================
* Following Code taken from ISA examples
* (c) Petr Matousek, 2020
*///========================================================================
int liten_for_packet(bool isVerbose){
  char errbuf[PCAP_ERRBUF_SIZE];  // constant defined in pcap.h
  pcap_t *handle;                 // packet capture handle 
  pcap_if_t *alldev, *dev ;       // a list of all input devices
  char *devname;                  // a name of the device
  struct in_addr a,b;
  bpf_u_int32 netaddr;            // network address configured at the input device
  bpf_u_int32 mask;               // network mask of the input device
  struct bpf_program fp;          // the compiled filter

  // open the input devices (interfaces) to sniff data
  if (pcap_findalldevs(&alldev, errbuf))
    err(1,"Can't open input device(s)");

  // list the available input devices
  printf("Available input devices are: ");
  for (dev = alldev; dev != NULL; dev = dev->next){
    printf("%s ",dev->name);
  }
  printf("\n");

  devname = alldev->name;  // select the name of first interface (default) for sniffing 
  
  // get IP address and mask of the sniffing interface
  if (pcap_lookupnet(devname,&netaddr,&mask,errbuf) == -1)
    err(1,"pcap_lookupnet() failed");

  a.s_addr=netaddr;
  printf("Opening interface \"%s\" with net address %s,",devname,inet_ntoa(a));
  b.s_addr=mask;
  printf("mask %s for listening...\n",inet_ntoa(b));

  // open the interface for live sniffing
  if ((handle = pcap_open_live(devname,BUFSIZ,1,1000,errbuf)) == NULL)
    err(1,"pcap_open_live() failed");

  // compile the filter
  if (pcap_compile(handle,&fp,"icmp or icmp6",0,netaddr) == -1)
    err(1,"pcap_compile() failed");
  
  // set the filter to the packet capture handle
  if (pcap_setfilter(handle,&fp) == -1)
    err(1,"pcap_setfilter() failed");

  // read packets from the interface in the infinite loop (count == -1)
  // incoming packets are processed by function mypcap_handler() 
  if (pcap_loop(handle,-1,mypcap_handler,NULL) == -1)
    err(1,"pcap_loop() failed");

  // close the capture device and deallocate resources
  pcap_close(handle);
  pcap_freealldevs(alldev);
  return 0;
}