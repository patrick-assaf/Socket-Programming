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

// data structure to store map information
typedef struct map {
    char map_id;
    double propagation_speed;
    double transmission_speed;
    int adj[M_SIZE][M_SIZE];
    int vertices[M_SIZE];
    int num_vertices;
    int num_edges;
} map_t;

// linked list nodes for the maps list
typedef struct node {
    map_t data;
    struct node *next;
} node_t;

// function that creates and allocates memory for a new map struct
map_t *createMap() {
    map_t *newMap = (map_t*)malloc(sizeof(map_t));
    if(!newMap) {
        perror("Error allocating map memory");
    }
    for(int i=0; i<M_SIZE; i++) {
        newMap->vertices[i] = 0;
    }
    for(int i=0; i<M_SIZE; i++) {
        for(int j=0; j<M_SIZE; j++) {
            newMap->adj[i][j] = 0;
        }
    }
    return newMap;
}

// function that creates and allocates memory for a new node struct
node_t *createNode(map_t *newMap) {
    node_t *newNode = (node_t*)malloc(sizeof(node_t));
    newNode->data = *newMap;
    newNode->next = NULL;
    if(!newNode) {
        perror("Error allocating node memory");
    }
    return newNode;
}

// method used to read and store data from the map file
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
            if(atoi(&buffer[0]) > 9 && atoi(&buffer[2]) > 9) {
                newMap->adj[i][j] = atoi(&buffer[5]);
                newMap->adj[j][i] = atoi(&buffer[5]);
            }
            else {
                newMap->adj[i][j] = atoi(&buffer[4]);
                newMap->adj[j][i] = atoi(&buffer[4]);
            }
        }
    }

    if(line > 0 && line <= 4) {
       return 0;
    }

    free(buffer);
    return 1;
}

// function used to identify the shortest paths within a map using Dijkstra's algorithm
int * dijkstra(int adj[M_SIZE][M_SIZE], int start, int *holder) {

    int s_distance[M_SIZE];
    int spt[M_SIZE];

    for(int i=0; i<M_SIZE; i++) {
        s_distance[i] = INT16_MAX;
        spt[i] = 0;
    }

    s_distance[start] = 0;
    
    for(int i=0; i<M_SIZE-1; i++) {
        int min_dist = INT16_MAX;
        int index;
        for(int i=0; i<M_SIZE; i++) {
            if(spt[i] == 0 && s_distance[i] <= min_dist) {
                min_dist = s_distance[i];
                index = i;
            }
        }
        spt[index] = 1;
        for(int j=0; j<M_SIZE; j++) {
            if (!spt[j] && adj[index][j] && s_distance[index] != INT16_MAX && (s_distance[index] + adj[index][j]) < s_distance[j]) {
                s_distance[j] = s_distance[index] + adj[index][j];
            }
        }
    }

    printf("The Server A has identified the following shortest paths:\n");
    printf("-----------------------------\n");
    printf("Destination \t Min Length\n");
    printf("-----------------------------\n");
    for (int i=0; i<M_SIZE; i++) {
        if(s_distance[i] != INT16_MAX && s_distance[i] != 0) {
            printf("%d \t\t %d\n", i, s_distance[i]);
            holder[i] = s_distance[i];
        }
        else {
            holder[i] = 0;
        }
    }
    printf("-----------------------------\n");

    return holder;
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
    printf("Map ID \t Num Vertices \t Num Edges\n");
    printf("-------------------------------------------\n");
    node_t *element = firstNode;
    while(element != NULL) {
        map_t *map = &element->data;
        printf("%c \t %d \t\t %d\n", map->map_id, map->num_vertices, map->num_edges);
        element = element->next;
    }
    printf("-------------------------------------------\n");

    while(1) {
        long buffer[3];
        socklen_t length;
        struct sockaddr_in client_address;

        // receiving the query information from the AWS server
        int receive = recvfrom(udp_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &client_address, &length);

        if(receive != -1) {
            printf("The Server A has received input for finding shortest paths: starting vertex %d of map %c.\n",
            (int)buffer[1], (int)buffer[0]);
        }

        // selecting map requested from query
        node_t *element = firstNode;
        map_t *picked_map = &firstNode->data;
        while(picked_map->map_id != (int)buffer[0]) {
            element = element->next;
            picked_map = &element->data;
        }

        // identifying shortest paths within map using Dijkstra's algorithm
        int holder[M_SIZE];
        int *shortest_path = dijkstra(picked_map->adj, (int)buffer[1], holder);

        int c_shortest_path[M_SIZE+2];
        for(int i=0; i<M_SIZE; i++) {
            c_shortest_path[i] = shortest_path[i];
        }
        int propagation = (int)((picked_map->propagation_speed)*100);
        int transmission = (int)((picked_map->transmission_speed)*100);
        c_shortest_path[M_SIZE] = propagation;
        c_shortest_path[M_SIZE+1] = transmission;

        // sending the shortest path information to the AWS server
        if(receive != -1) {
            socklen_t a_addr_size = sizeof s_address;
            int udp_send = sendto(udp_socket, c_shortest_path, sizeof(c_shortest_path), 0, (struct sockaddr *) &s_address, a_addr_size);
            if(udp_send == -1) {
                perror("Error sending data to AWS");
            }
            else {
                printf("The Server A has sent shortest paths to AWS.\n");
            }
        }
        
    }

    return 0;
}