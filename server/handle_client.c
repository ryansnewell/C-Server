//
//  handle_client.c
//  server
//
//  Created by TheKingDoof on 4/25/15.
//

#include "handle_client.h"
#include "socket_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>
//#include <sys/sendfile.h> //linux
#include <netinet/in.h>

#define SERVERNAME "DOOFS SERVER/0.0.5\n"

// TODO add support for parameters
typedef struct {
    int returncode;
    char *filename;
    char *ext;
    char *server;
    char *type;
    char *transfer_encoding;
} httpRequest;



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

        //char *type = malloc(sizeof(char) * 18);
        //char *parameter = malloc(sizeof(char) * 18);

        //type = strtok(tmp, ":");
        //parameter = strtok(NULL, "");


        char *type = malloc(sizeof(char) * 18);
        char *parameter = malloc(sizeof(char) * 18);

        type = strtok(tmp, ":");
        parameter = strtok(NULL, "");

        block = realloc(block, size+oldsize);
        oldsize += size;
        strcat(block, tmp);
    }

    free(tmp);
    //tmp = NULL;

    printf("Not in getMessage()\n");


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

    //gets file name from msg, puts into file
    sscanf(msg, "GET %s HTTP/1.1", file);

    //accepts a quit command

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

    //if theres an extension puts its location in ext
    ext = strrchr(file, '.');

    //if no ext then assume html
    // TODO assume lua instead
    if(ext == NULL && strcmp(file, "/") != 0)
    {
        ext = ".html";
        strcat(file, ext);
    }

    //add html to the front to insure it checks the html folder
    // TODO maybe ph = lua if lua is ext?

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

    ret.server = SERVERNAME;

    if(test != NULL)
    {
        ret.returncode = 400;
        ret.filename   = "html/400.html";
        ret.ext        = ".html";
        ret.type       = "text/html\n";
    } else if(test2 == 0) {
        ret.returncode = 200;
        ret.filename   = "html/index.html";
        ret.ext        = ".html";
        ret.transfer_encoding = NULL;
        ret.type       = "text/html\n";
    } else if (exists != NULL) {
        ret.returncode = 200;
        ret.filename   = filename;
        char *buff = strtok(filename, ".");
        char *ext = strtok(NULL, "");
        free(buff);
        ret.ext        = ext;

        printf("The extension is %s\n", buff);
        if(strcmp(buff, ".html") != 0){
            ret.type = "text/html\n";
            ret.transfer_encoding = NULL;
        }
        else {
            ret.type = strcat("image/", buff);
            ret.transfer_encoding = "binary\n";
        }

        fclose(exists);
    } else {
        ret.returncode = 404;
        ret.filename   = "html/404.html";
        ret.ext        = ".html";
        ret.type       = "text/html\n";
    }

    printf("Return code: %d, filename: %s\n", ret.returncode, ret.filename);
    return ret;
}

int printHeader(httpRequest details, int* fd)
{

    char *header200 = "HTTP/1.0 200 OK\nServer: myserver v0.1\nContent-Type: image/jpg\nContent-Transfer-Encoding: binary\n\n";
    //char *headerStart = "HTTP/1.0 ";
    //char *headerEnd   = "\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    //char *hd = malloc(sizeof(headerStart) + sizeof(headerEnd) + 30);

    char *header200 = "HTTP/1.0 200 OK\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *header400 = "HTTP/1.0 400 Bad Request\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *header404 = "HTTP/1.0 404 Not Found\nServer: myserver v0.1\nContent-Type: text/html\n\n";
    char *hd = malloc(sizeof(char) * 200);
    hd = strcpy(hd, "HTTP/1.0 ");
    printf("printHeader(code = %d)\n", details.returncode);
    switch (details.returncode) {
        case 200:
            hd = strcat(hd, "200 OK\n");
            printf("Header set to %d\n", details.returncode);
            break;

        case 400:
            hd = strcat(hd, "400 Bad Request\n");
            printf("Header set to %d\n", details.returncode);
            break;

        case 404:
            hd = strcat(hd, "404 Not Found\n");
            printf("Header set to %d\n", details.returncode);
            break;

        default:
            return 0;
            break;
    }

    printf("DEBUG\n");

    hd = strcat(hd, "Server: ");
    hd = strcat(hd, details.server);
    hd = strcat(hd, "Content-Type: ");
    hd = strcat(hd, details.type);
    if(details.transfer_encoding)
    {
        hd = strcat(hd, "Content-Transfer-Encoding: ");
        hd = strcat(hd, details.transfer_encoding);
    }

    hd = strcat(hd, "\n");
    printf("hd = %s\n", hd);
    sendMessage(hd, &(*fd));
    return (int)strlen(hd);
}

int printFile(char *filename, int* fd)
{
/*    FILE *read;
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

    return totalsize;*/

    struct stat stat_buf;
    off_t offset = 0;

    int file_desc = open(filename, O_RDONLY);
    if(file_desc == -1)
    {
        printf("Error opening requested file\n");
        handleExit(EXIT_FAILURE, fd);
    }

    int frc = fstat(file_desc, &stat_buf);
    if(frc != 0)
    {
        printf("FRC != 0");
    }


    int rc = sendfile(file_desc, *fd, offset, &stat_buf.st_size, NULL, 0);
    if(rc == -1)
    {
        printf("Error sending file %s\n", filename);
        handleExit(EXIT_FAILURE, fd);
    }
    close(file_desc);
    return (int)stat_buf.st_size;

    return totalsize;
}

void * handle_http(void * p_clientfd)
{
    int clientfd = *(int*)p_clientfd;
    free((int*)p_clientfd);

    printf("Getting Message\n");
    char * header = getMessage(&clientfd);

    httpRequest details = parseRequest(header, &clientfd);

    int headersize = printHeader(details, &clientfd);

    int headersize = printHeader(details.returncode, &clientfd);
    int pagesize   = printFile(details.filename, &clientfd);

    printf("Headersize: %d\nPagesize: %d\n", headersize, pagesize);

    free(header);
    close(clientfd);

    return 0;
}
