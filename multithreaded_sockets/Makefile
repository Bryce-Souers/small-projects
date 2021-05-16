all:    server client

client: client.c
	gcc -g client.c -o client  -lnsl -lpthread

server: server.c
	gcc -g server.c -o server  -lnsl -lpthread

clean:
	rm client server

