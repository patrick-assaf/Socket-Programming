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
    float propagation_speed;
    float transmission_speed;
    int adj[20][20];
    int vertices[20];
    int num_vertices;
    int num_edges;
} map_t;

// creating linked list nodes for the maps list
typedef struct node {
    map_t data;
    struct node *next;
} node_t;

map_t *createMap() {
    map_t *newMap = (map_t*)malloc(sizeof(map_t));
    if(!newMap) {
        perror("Error allocating map memory");
    }
    return newMap;
}

node_t *createNode(map_t *newMap) {
    node_t *newNode = (node_t*)malloc(sizeof(node_t));
    newNode->data = *newMap;
    newNode->next = NULL;
    if(!newNode) {
        perror("Error allocating node memory");
    }
    return newNode;
}

int new = 1;
int num_of_maps = 0;
ssize_t line = 0;
size_t buffer_size = 0;
char *buffer = NULL;

int readFromMapFile(map_t *newMap, FILE *mapFile) {

    num_of_maps += 1;

    if(new == 1) {
        line = getline(&buffer, &buffer_size, mapFile);
        newMap->map_id = *buffer;
    }
    else if(new == 0) {
        newMap->map_id = *buffer;
    }

    line = getline(&buffer, &buffer_size, mapFile);
    newMap->propagation_speed = atof(buffer);

    line = getline(&buffer, &buffer_size, mapFile);
    newMap->transmission_speed = atof(buffer);

    while(line >= 4) {
        line = getline(&buffer, &buffer_size, mapFile);
        if(line > 4) {
            newMap->num_edges += 1;
            int i = atoi(&buffer[0]);
            int j = atoi(&buffer[2]);
            if(newMap->vertices[i] != 1) {
                newMap->vertices[i] = 1;
                newMap->num_vertices += 1;
            }
            if(newMap->vertices[j] != 1) {
                newMap->vertices[j] = 1;
                newMap->num_vertices += 1;
            }
            newMap->adj[i][j] = atoi(&buffer[4]);
            newMap->adj[j][i] = atoi(&buffer[4]);
        }
    }

    if(line > 0 && line <= 4) {
       return 0;
    }

    free(buffer);
    return 1;
}

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

    // reading and storing data from map.txt
    FILE *mapFile = fopen("map.txt", "r");
    if(!mapFile) {
        perror("Error opening file");
    }

    map_t *firstMap = createMap();
    int done = readFromMapFile(firstMap, mapFile);
    node_t *firstNode = createNode(firstMap);

    while(done == 0) {
        new = 0;
        map_t *nextMap = createMap();
        done = readFromMapFile(nextMap, mapFile);
        node_t *nextNode = createNode(nextMap);
        node_t *element = firstNode;
        while(element->next != NULL) {
            element = element->next;
        }
        element->next = nextNode;
    }

    fclose(mapFile);

    printf("The Server A has constructed a list of %d maps:\n", num_of_maps);
    printf("-------------------------------------------\n");
    printf("Map ID Num Vertices Num Edges\n");
    printf("-------------------------------------------\n");
    node_t *element = firstNode;
    while(element != NULL) {
        map_t *map = &element->data;
        printf("%c      %d      %d\n", map->map_id, map->num_vertices, map->num_edges);
        element = element->next;
    }
    printf("-------------------------------------------\n");

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