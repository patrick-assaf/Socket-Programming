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

int main() {

    // booting up message
    printf("The AWS is up and running.\n");

    char shortest_path[256] = "This is the shortest path: ~"; // replace later with real data

    // creating a TCP socket to connect to the Client
    int aws_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(aws_socket == -1) {
        perror("Error creating TCP socket");
    }

    // creating a UDP socket to communicate with servers A & B
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1) {
        perror("Error creating UDP socket");
    }

    // initializing the elements of the AWS server TCP socket address information
    struct sockaddr_in s_address;
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(24128);
    s_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // initializing the elements of the AWS server UDP socket address information
    struct sockaddr_in udp_address;
    udp_address.sin_family = AF_INET;
    udp_address.sin_port = htons(23128);
    udp_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // initializing the elements of the Server A UDP socket address information
    struct sockaddr_in a_address;
    a_address.sin_family = AF_INET;
    a_address.sin_port = htons(21128);
    a_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // initializing the elements of the Client socket address information
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    s_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binding the AWS server TCP socket to the IP address and port number
    int tcp_bind = bind(aws_socket, (struct sockaddr *) &s_address, sizeof(s_address));
    if(tcp_bind == -1) {
        perror("Error binding TCP socket with address information");
    }

    // binding the AWS server UDP socket to the IP address and port number
    int udp_bind = bind(udp_socket, (struct sockaddr *) &udp_address, sizeof(udp_address));
    if(udp_bind == -1) {
        perror("Error binding UDP socket with address information");
    }

    // listening for connections
    int listening = listen(aws_socket, 3);
    if(listening == -1) {
        perror("Error listening to TCP port");
    }

    while(1) {
        socklen_t client_addr_size = sizeof client_address;
        int client_fd = accept(aws_socket, (struct sockaddr *) &client_address, &client_addr_size);

        // receiving the query information from the Client
        char buffer[3];
        int receive = recv(client_fd, &buffer, sizeof(buffer), 0);
        if(receive != -1) {
            printf("The AWS has received map ID %c, start vertex %d and file size %d from the client using TCP over port %d.\n",
            buffer[0], buffer[1], buffer[2], ntohs(s_address.sin_port));
        }
        else if(client_fd != -1 && receive == -1) {
            perror("Error receiving data from Client");
        }

        struct query_t {
            char map_id;
            int start_index, file_size;
        };

        struct query_t query;

        query.map_id = buffer[0];
        query.start_index = buffer[1];
        query.file_size = buffer[2];

        // sending data to Server A
        if(client_fd != -1 && receive != -1) {
            socklen_t a_addr_size = sizeof a_address;
            int udp_send = sendto(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &a_address, a_addr_size);
            if(udp_send == -1) {
                perror("Error sending data to Server A");
            }
            else {
                printf("The AWS has sent map ID and starting vertex to server A using UDP over port %d.\n", ntohs(udp_address.sin_port));
            }
        }
        
        // sending data to the Client
        int sending = send(client_fd, shortest_path, sizeof(shortest_path), 0);
        if(sending != -1) {
            printf("The AWS has sent calculated delay to client using TCP over port %d.\n", ntohs(s_address.sin_port));
        }
        else if(client_fd != -1 && sending == -1) {
            perror("Error sending data to Client");
        }

        sleep(1);
    }

    // closing the socket
    close(aws_socket);

    return 0;
}