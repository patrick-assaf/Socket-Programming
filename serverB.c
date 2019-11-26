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

    // creating a UDP socket to communicate with AWS
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1) {
        perror("Error creating UDP socket");
    }

    // initializing the elements of the Server B UDP socket address information
    struct sockaddr_in udp_address;
    udp_address.sin_family = AF_INET;
    udp_address.sin_port = htons(22128);
    udp_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // initializing the elements of the AWS server UDP socket address information
    struct sockaddr_in s_address;
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(23128);
    s_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binding the UDP socket to the IP address and port number
    int udp_bind = bind(udp_socket, (struct sockaddr *) &udp_address, sizeof(udp_address));
    if(udp_bind == -1) {
        perror("Error binding UDP socket with address information");
    }

    printf("The Server B is up and running using UDP on port %d.\n", ntohs(udp_address.sin_port));

    while(1) {
        long buffer[M_SIZE+3];
        socklen_t length;
        struct sockaddr_in client_address;

        // receiving the data for calculation from the AWS server
        int receive = recvfrom(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &client_address, &length);

        if(receive != -1) {
            printf("The Server B has received data for calculation:\n");
        }

        double propagation_speed = ((double)buffer[M_SIZE])/100;
        double transmission_speed = ((double)buffer[M_SIZE+1])/100;
        long file_size = buffer[M_SIZE+2]/8;
        printf("* Transmission speed: %.2f Bytes/s;\n* Propagation speed: %.2f km/s;\n", transmission_speed, propagation_speed);
        for(int i=0; i<M_SIZE; i++) {
            if(buffer[i] != 0) {
                printf("* Path length for destination %d: %d km;\n", i, (int)buffer[i]);
            }
        }

        // computing the transmission delay, propagation delay, and end-to-end delay
        float calculations[M_SIZE][3];
        for(int i=0; i<M_SIZE; i++) {
            if(buffer[i] != 0) {
                calculations[i][0] = ((float)file_size)/transmission_speed;
                calculations[i][1] = ((float)buffer[i])/propagation_speed;
                calculations[i][2] = calculations[i][0] + calculations[i][1];
            }
            else {
                calculations[i][0] = 0;
                calculations[i][1] = 0;
                calculations[i][2] = 0;
            }
        }

        printf("The Server B has finished the calculation of the delays:\n");
        printf("-------------------------------\n");
        printf("Destination \t Delay\n");
        printf("-------------------------------\n");
        for (int i=0; i<M_SIZE; i++) {
            if(buffer[i] != 0) {
                printf("%d \t\t %.2f \n", i, calculations[i][2]);
            }
        }
        printf("-------------------------------\n");

        // sending the calculations to AWS
        if(receive != -1) {
            socklen_t s_addr_size = sizeof s_address;
            int udp_send = sendto(udp_socket, calculations, sizeof(calculations), 0, (struct sockaddr *) &s_address, s_addr_size);
            if(udp_send == -1) {
                perror("Error sending data to Server A");
            }
            else {
                printf("The Server B has finished sending the output to AWS.\n");
            }
        }

        sleep(1);
    }

    return 0;
}