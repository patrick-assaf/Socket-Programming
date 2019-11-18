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

int main(int argc, char* argv[]) {

    // defining the variables that will hold the query information
    struct query_t {
        char map_id;
        int start_index, file_size;
    };

    struct query_t query;
    query.map_id = *argv[1];
    query.start_index = atoi(argv[2]);
    query.file_size = atoi(argv[3]);

    // booting up message
    printf("The client is up and running.\n");

    // creating a TCP socket to connect to the AWS server
    int aws_socket = socket(AF_INET, SOCK_STREAM, 0);

    // initializing the elements of the socket address information 
    struct sockaddr_in s_address;
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(24128); // change it later to dynamic port
    s_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // establishing a connection and checking for error
    int conn_status = connect(aws_socket, (struct sockaddr *) &s_address, sizeof(s_address));
    if(conn_status == -1) {
        printf("Connection to AWS server could not be made...\n");
    }

    // sending the query information to the AWS server
    char buffer[3];
    buffer[0] = query.map_id;
    buffer[1] = query.start_index;
    buffer[2] = query.file_size;

    send(aws_socket, buffer, sizeof(buffer), 0);

    // receiving the shortest path information back from the AWS server
    char shortest_path[256]; // change data structure later
    recv(aws_socket, &shortest_path, sizeof(shortest_path), 0);

    printf("Arguments are: %c %d %d\n", buffer[0], buffer[1], buffer[2]);

    printf("Information received from server: %s\n", shortest_path);

    // closing the socket
    close(aws_socket);

    return 0;
}