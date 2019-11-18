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

    // initializing the elements of the AWS server socket address information
    struct sockaddr_in s_address;
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(24128);
    s_address.sin_addr.s_addr = inet_addr("127.0.0.1");
        
    // initializing the elements of the Client socket address information
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(25128);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t addr_size = sizeof client_address;

    // binding the AWS server TCP socket to the IP address and port number
    bind(aws_socket, (struct sockaddr *) &s_address, sizeof(s_address));

    // listening for connections and accepting the connection with the CLient 
    listen(aws_socket, 3);

    int client_fd = accept(aws_socket, (struct sockaddr *) &client_address, &addr_size);

    // receiving the query information from the Client
    char buffer[3];
    recv(client_fd, &buffer, sizeof(buffer), 0);

    struct query_t {
        char map_id;
        int start_index, file_size;
    };

    struct query_t query;

    query.map_id = buffer[0];
    query.start_index = buffer[1];
    query.file_size = buffer[2];

    printf("The AWS has received map ID %c, start vertex %d and file size %d from the client using TCP over port %d.\n",
        buffer[0], buffer[1], buffer[2], ntohs(s_address.sin_port));
    
    // sending data to the Client
    send(client_fd, shortest_path, sizeof(shortest_path), 0);

    // closing the socket
    close(aws_socket);

    return 0;
}