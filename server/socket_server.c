//
//  socket_server.c
//  server
//
//  Created by TheKingDoof on 4/25/15.
//

#include "socket_server.h"
#include "handle_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <pthread.h>


int serverfd;

void exitServer(int code)
{
    exit(code);
}

int main()
{
    struct sockaddr_in serv_addr, cli_addr;

    pthread_t thread;

    int addrlen;
    int iSetOption = 1;
    int * p_clientfd;
    int clilen = sizeof(cli_addr);

    (void) signal(SIGINT, exitServer);

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char *) &iSetOption, sizeof(iSetOption));

    if(serverfd == 0)
    {
        perror("Error creating socket");
        exitServer(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(1324);


    if( bind(serverfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Binding problem");
        exitServer(EXIT_FAILURE);
    }

    if( listen(serverfd, 5) == -1)
    {
        perror("Listening problem");
        exitServer(EXIT_FAILURE);
    }
    printf("Listening...\n");

    addrlen = sizeof(serv_addr);

    while (1) {
        p_clientfd = malloc(sizeof(int));
        *p_clientfd = accept(serverfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);
        pthread_create(&thread, NULL, &handle_http, p_clientfd);
        pthread_detach(thread);
    }

    return EXIT_SUCCESS;
}
