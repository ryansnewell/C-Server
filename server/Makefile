main : main.c
	gcc -o main.o main.c

webserver : handle_client.c socket_server.c
	gcc -o webserver.o socket_server.c handle_client.c
