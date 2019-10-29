//
// Created by Jaimie on 29-10-19.
//


// C program to Implement Ping

// compile as -o ping
// run as sudo ./ping <hostname>

#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <netinet/icmp6.h>
#include <fcntl.h>
#include <csignal>
#include <ctime>
#include <upnp/upnp.h>

// Define the Packet Constants
// ping packet size
#define PING_PKT_S 8

// Automatic port number
#define PORT_NO 0

// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

// Define the Ping Loop
int pingloop=1;


// ping packet structure
struct icmp6_hdr hdr;

// Calculating the Check Sum
unsigned short checksum(void *b, int len) {
    unsigned short *buff = (unsigned short*) b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buff++;
    if ( len == 1 )
        sum += *(unsigned char*) b;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


// Interrupt handler
void intHandler(int dummy) {
    pingloop=0;
}

// make a ping request
void send_ping(int ping_sockfd, struct sockaddr_in6 *ping_addr) {
    int ttl_val=64, msg_count=0, i, addr_len;

    struct icmp6_hdr pckt;
    struct sockaddr_in r_addr;
    struct timeval tv_out;
    tv_out.tv_sec = 0;
    tv_out.tv_usec = 1;


    // setting timeout of recv setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO,
               (const char*)&tv_out, sizeof tv_out);

    // send icmp packet in an infinite loop
    while(pingloop) {
        // flag is whether packet was sent or not

        //filling packet
        bzero(&pckt, sizeof(pckt));

        pckt.icmp6_type = ICMP6_ECHO_REQUEST;
        pckt.icmp6_code = 0;
        pckt.icmp6_dataun.icmp6_un_data32[0] = 0;
        pckt.icmp6_cksum = checksum(&pckt, sizeof(pckt));

        //send packet
        if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0,
                    (struct sockaddr*) ping_addr,
                    sizeof(*ping_addr)) <= 0) {
            printf("\nPacket Sending Failed! %s\n");
        }


        //receive packet
        addr_len=sizeof(r_addr);

        if (recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr,
                     reinterpret_cast<socklen_t *>(&addr_len)) <= 0 && msg_count > 1) {
            printf("\nPacket receive failed!\n");
        }
    }
}

// Driver Code
int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in6 addr_con;

    addr_con.sin6_addr = {0x20, 0x01, 0x06, 0x10, 0x19, 0x08, 0xa0, 0x00, 0x00, 0x19, 0x00, 0x19, 0xff, 0xd1, 0x00, 0xff};
    addr_con.sin6_family = AF_INET6;

    sockfd = socket(AF_INET6, SOCK_RAW | SOCK_NONBLOCK, IPPROTO_ICMPV6);
    if(sockfd < 0) {
        printf("\nSocket file descriptor not received!!\n");
        return 0;
    }
    else
        printf("\nSocket file descriptor %d received\n", sockfd);

    signal(SIGINT, intHandler);//catching interrupt

    //send pings continuously
    send_ping(sockfd, &addr_con);

    return 0;
}
