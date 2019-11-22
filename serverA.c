#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

// creating a data structure to store map information
typedef struct map {
    char map_id;
    int propagation_speed;
    int transmission_speed;
    int adj[10][10];
} map_t;

// creating linked list nodes for the maps list
typedef struct node {
    map_t data;
    struct node *next;
} node_t;

int main() {

    // creating a UDP socket to communicate with AWS
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1) {
        perror("Error creating UDP socket");
    }

    // initializing the elements of the Server A UDP socket address information
    struct sockaddr_in udp_address;
    udp_address.sin_family = AF_INET;
    udp_address.sin_port = htons(21128);
    udp_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binding the UDP socket to the IP address and port number
    int udp_bind = bind(udp_socket, (struct sockaddr *) &udp_address, sizeof(udp_address));
    if(udp_bind == -1) {
        perror("Error binding UDP socket with address information");
    }

    printf("The Server A is up and running using UDP on port %d.\n", ntohs(udp_address.sin_port));

    while(1) {
        char buffer[3];
        socklen_t length;
        struct sockaddr_in client_address;
        int receive = recvfrom(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &client_address, &length);

        if(receive != -1) {
            printf("The Server A has received input for finding shortest paths: starting vertex %d of map %c.\n",
            buffer[1], buffer[0]);
        }
    }

    return 0;
}