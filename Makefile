
.PHONY: aws
.PHONY: serverA
.PHONY: serverB	

all: client.o aws.o serverA.o serverB.o
	gcc -o client -g client.o
	gcc -o aws -g aws.o
	gcc -o serverA -g serverA.o
	gcc -o serverB -g serverB.o
aws.o: aws.c
	gcc -g -c -Wall aws.c
client.o: client.c
	gcc -g -c -Wall client.c
serverA.o: serverA.c
	gcc -g -c -Wall serverA.c
serverB.o: serverB.c
	gcc -g -c -Wall serverB.c

aws:
	./aws
serverA:
	./serverA	
serverB:
	./serverB

clean:
	rm -f *.o aws
	rm -f *.o client
	rm -f *.o serverA
	rm -f *.o serverB

