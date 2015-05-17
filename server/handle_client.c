//
//  handle_client.c
//  server
//
//  Created by TheKingDoof on 4/25/15.
//  Copyright (c) 2015 Doofopolis. All rights reserved.
//

#include "handle_client.h"
#include "socket_server.h"

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

typedef struct {
    int returncode;
    char *filename;
} httpRequest;

char *connection;

void handleExit(int code, int* fd)
{
    close(*fd);
    exit(code);
}

char *getMessage(int* fd)
{
    FILE *sstream;
    
    if( (sstream = fdopen(*fd, "r")) == NULL)
    {
        perror("Error Sstream thing");
        handleExit(EXIT_FAILURE, fd);
    }
    
    size_t size = 1;
    
    char *block;
    
    if( (block = malloc(sizeof(char) * size)) == NULL)
    {
        perror("Error allocating memory for block");
        handleExit(EXIT_FAILURE, fd);
    }
    
    *block = '\0';
    char *tmp;
    
    if( (tmp = malloc(sizeof(char) * size)) == NULL)
    {
        perror("Error allocating tmp");
        handleExit(EXIT_FAILURE, fd);
    }
    
    *tmp = '\0';
    
    int end;
    int oldsize = 1;
    
    while( (end = (int)getline(&tmp, &size, sstream)) > 0)
    {
        if( strcmp(tmp, "\r\n") == 0 )
        {
            break;
        }
        
        char *type = malloc(sizeof(char) * 18);
        char *parameter = malloc(sizeof(char) * 18);
        
        type = strtok(tmp, ":");
        parameter = strtok(NULL, "");
        
        block = realloc(block, size+oldsize);
        oldsize += size;
        strcat(block, tmp);
    }
    
    free(tmp);
    
    return block;
}

int sendMessage(char *msg, int* fd)
{
    return (int)write(*fd, msg, strlen(msg));
}

char * getFileName(char* msg, int* fd)
{
    char * file;
    
    if( (file = malloc(sizeof(char) * strlen(msg))) == NULL)
    {
        perror("Error allocating file");
        handleExit(EXIT_FAILURE, fd);
    }
    
    sscanf(msg, "GET %s HTTP/1.1", file);
    
    if(strcmp(file, "quit") == 0)
    {
        handleExit(EXIT_SUCCESS, fd);
    }
    
    char *base;
    if( (base = malloc(sizeof(char) * strlen(file) + 18)) == NULL)
    {
        perror("Error allocating base");
        handleExit(EXIT_FAILURE, fd);
    }
    
    char * ext = malloc(sizeof(char) * strlen(file));
    ext = strrchr(file, '.');
    if(ext == NULL && strcmp(file, "/") != 0)
    {
        ext = ".html";
        strcat(file, ext);
    }
    
    char *ph = "html";
    strcpy(base, ph);
    strcat(base, file);
    
    free(file);
    
    return base;
    
}

httpRequest parseRequest(char *msg, int* fd)
{
    httpRequest ret;
    
    char * filename;
    if( (filename = malloc(sizeof(char) * strlen(msg))) == NULL)
    {
        perror("Error allocating for filename");
        handleExit(EXIT_FAILURE, fd);
    }
    
    filename = getFileName(msg, fd);
    
    char *badstring = "..";
    char *test = strstr(filename, badstring);
    
    int test2 = strcmp(filename, "html/");
    
    FILE *exists = fopen(filename, "r");
    
    if(test != NULL)
    {
        ret.returncode = 400;
        ret.filename   = "html/400.html";
    } else if(test2 == 0) {
        ret.returncode = 200;
        ret.filename   = "html/index.html";
    } else if (exists != NULL) {
        ret.returncode = 200;
        ret.filename   = filename;
        fclose(exists);
    } else {
        ret.returncode = 404;
        ret.filename   = "html/404.html";
    }
    
    printf("Return code: %d, filename: %s\n", ret.returncode, ret.filename);
    return ret;
}

int printHeader(int returncode, int* fd)
{
    //char *headerStart = "HTTP/1.0 ";
    //char *headerEnd   = "\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    //char *hd = malloc(sizeof(headerStart) + sizeof(headerEnd) + 30);
    
    char *header200 = "HTTP/1.0 200 OK\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *header400 = "HTTP/1.0 400 Bad Request\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *header404 = "HTTP/1.0 404 Not Found\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *hd;
    printf("printHeader(code = %d)\n", returncode);
    switch (returncode) {
        case 200:
            hd = header200;
            printf("Header set to %d\n", returncode);
            break;
            
        case 400:
            hd = header400;
            printf("Header set to %d\n", returncode);
            break;
            
        case 404:
            hd = header404;
            printf("Header set to %d\n", returncode);
            break;
            
        default:
            return 0;
            break;
    }
    sendMessage(hd, &(*fd));
    return (int)strlen(hd);
}

int printFile(char *filename, int* fd)
{
    FILE *read;
    if( (read = fopen(filename, "r")) == NULL)
    {
        printf("Error allocating read\n");
        handleExit(EXIT_FAILURE, fd);
    }
    
    int totalsize;
    struct stat st;
    stat(filename, &st);
    totalsize = (int)st.st_size;
    
    size_t size = 1;
    
    char *temp;
    if( (temp = malloc(sizeof(char) * size)) == NULL)
    {
        printf("Error allocating temp in printFile\n");
        handleExit(EXIT_FAILURE, fd);
    }
    
    int end;
    while ( (end = (int)getline(&temp, &size, read)) > 0) {
        //printf("%s", temp);
        sendMessage(temp, &(*fd));
    }
    
    sendMessage("\n", &(*fd));
    
    free(temp);
    
    return totalsize;
}

void * handle_http(void * p_clientfd)
{
    int clientfd = *(int*)p_clientfd;
    free((int*)p_clientfd);
    
    printf("Getting Message\n");
    char * header = getMessage(&clientfd);
    
    httpRequest details = parseRequest(header, &clientfd);
    
    int headersize = printHeader(details.returncode, &clientfd);
    int pagesize   = printFile(details.filename, &clientfd);
    
    printf("Headersize: %d\nPagesize: %d\n", headersize, pagesize);
    
    free(header);
    close(clientfd);
    
    return 0;
}






