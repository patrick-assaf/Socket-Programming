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

#define M_SIZE 20

int main() {

    // booting up message
    printf("The AWS is up and running.\n");

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

    // initializing the elements of the Server B UDP socket address information
    struct sockaddr_in b_address;
    b_address.sin_family = AF_INET;
    b_address.sin_port = htons(22128);
    b_address.sin_addr.s_addr = inet_addr("127.0.0.1");

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
        long buffer[3];
        int receive = recv(client_fd, &buffer, sizeof(buffer), 0);
        if(receive != -1) {
            printf("The AWS has received map ID %c, start vertex %ld and file size %ld from the client using TCP over port %d.\n",
            (int)buffer[0], buffer[1], buffer[2], ntohs(s_address.sin_port));
        }
        else if(client_fd != -1 && receive == -1) {
            perror("Error receiving data from Client");
        }

        // sending the query information to Server A
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

        sleep(1);

        // receiving the shortest path information from Server A
        int buffer_a[M_SIZE+2];
        socklen_t length_a;
        struct sockaddr_in recv_a_address;

        int receive_a = recvfrom(udp_socket, &buffer_a, sizeof(buffer_a), 0, (struct sockaddr *) &recv_a_address, &length_a);

        if(receive_a != -1) {
            printf("The AWS has received shortest path from server A:\n");
            printf("-----------------------------\n");
            printf("Destination \t Min Length\n");
            printf("-----------------------------\n");
            for (int i=0; i<M_SIZE; i++) {
                if(buffer_a[i] != 0) {
                    printf("%d \t\t %d\n", i, buffer_a[i]);
                }
            }
            printf("-----------------------------\n");
        }

        // sending data for calculation to Server B
        long buffer_b[M_SIZE+3];
        for(int i=0; i<M_SIZE+2; i++) {
            buffer_b[i] = buffer_a[i];
        }
        buffer_b[M_SIZE+2] = buffer[2];

        if(receive_a != -1) {
            socklen_t b_addr_size = sizeof b_address;
            int b_send = sendto(udp_socket, buffer_b, sizeof(buffer_b), 0, (struct sockaddr *) &b_address, b_addr_size);
            if(b_send == -1) {
                perror("Error sending data to Server B");
            }
            else {
                printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port %d.\n", ntohs(udp_address.sin_port));
            }
        }

        sleep(1);

        // receiving output from Server B
        float calculations[M_SIZE][3];
        socklen_t length_b;
        struct sockaddr_in recv_b_address;

        int receive_b = recvfrom(udp_socket, &calculations, sizeof(calculations), 0, (struct sockaddr *) &recv_b_address, &length_b);

        float results[M_SIZE][4];

        if(receive_b != -1){
            for(int i=0; i<M_SIZE; i++) {
                results[i][0] = buffer_a[i];
                results[i][1] = calculations[i][0];
                results[i][2] = calculations[i][1];
                results[i][3] = calculations[i][2];
            }
            printf("The AWS has received delays from server B:\n");
            printf("----------------------------------------------------------\n");
            printf("Destination \t Tt \t\t Tp \t\t Delay\n");
            printf("----------------------------------------------------------\n");
            for (int i=0; i<M_SIZE; i++) {
                if(results[i][0] != 0) {
                    printf("%d \t\t %.2f \t %.2f \t\t %.2f \n", i, results[i][1], results[i][2], results[i][3]);
                }
            }
            printf("----------------------------------------------------------\n");
        }

        // sending data to the Client
        if(receive_b != -1) {
            int sending = send(client_fd, results, sizeof(results), 0);
            if(sending != -1) {
                printf("The AWS has sent calculated delay to client using TCP over port %d.\n", ntohs(s_address.sin_port));
            }
            else if(client_fd != -1 && sending == -1) {
                perror("Error sending data to Client");
            }
        }
    }

    // closing the socket
    close(aws_socket);

    return 0;
}