# Socket-Programming
UNIX socket programming project

## Makefile commands: 
```
$ make all          # compile all source files seperately and create executables.  
$ make serverA      # run the Server A program.  
$ make serverB      # run the Server B program.  
$ make aws          # run the AWS server program.  
$ make clean        # delete all binary files and executables within the project directory.
```

## Command to run the client program:
```
$ ./client [Map ID] [Source Vertex Index] [File Size]
```

- After booting up, the client establishes a TCP connection with AWS. After successfully establishing the connection, the client sends the input (map ID, source vertex index and file size) to AWS. 

- The AWS server then sends selected input value(s) to the backend servers A and B, depending on their functionalities. The communication between the AWS server and both the backend servers is via UDP.

- Afterwards, server A will execute the "map construction" operation and read the map data file (map.txt). Reading this file allows A to construct a list of maps. Every map will be identified by its unique ID. The backend server A will keep this list so that the client can query on any map anytime in the future.

- Once AWS receives all required data from the client, it forwards the query from the client to the backend server A. After server A receives the parameters, it will perform the "path finding" operation using Dijkstra's algorithm to find the shortest path from the requested source vertex to all other nodes in the same map. Then, server A will send all required map information back to AWS.

- When the AWS receives the corresponding map information from server A, it forwards both the result and the file size to the backend server B. The backend server B is a computing server. It performs the operations based on the data sent by the AWS server. With the given file size, server B computes the transmission delay, the propagation delay, and the end-to-end delay for the start node to send the file to all other nodes in the same map.

- Those results are sent back to the AWS using UDP. When the AWS receives the calculation results, it forwards all the results to the client using TCP. The results include the minimum path length to all destination nodes on the selected map and the 3 delays to transfer the file to each corresponding destination.
