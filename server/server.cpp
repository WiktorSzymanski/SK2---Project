#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define BACKLOG_SIZE 5
#define SERVER_PORT 1234

struct threadData
{
    int clientFd;
    struct sockaddr_in clientAddress;
};

void *thread_start(void *arg)
{
    struct threadData *d = (struct threadData *)arg;

    char buffer[BUFFER_SIZE] = "";
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";

    printf("\e[32m[CONNECTED]\e[0m: %s\n\e[33m[MESSAGE]\e[0m: ", inet_ntoa((struct in_addr)d->clientAddress.sin_addr));

    while(1){
    read(d->clientFd, buffer, BUFFER_SIZE);

    printf("%s\n\n", buffer);

    write(d->clientFd, buffer,  BUFFER_SIZE);
    }
    
    
    close(d->clientFd);
    free(d);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    socklen_t clientSocketLength;
    struct sockaddr_in serverAddress;

    int serverFd = socket(PF_INET, SOCK_STREAM, 0),
        on = 1;

    pthread_t tid;

    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    int isPortAlreadyTaken = -1 == bind(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    if (isPortAlreadyTaken)
    {
        printf("Couldn't open socket.\n");
        return EXIT_FAILURE;
    }

    listen(serverFd, BACKLOG_SIZE);

    while (1)
    {
        struct threadData *acceptData = (threadData*)malloc(sizeof(struct threadData));
        clientSocketLength = sizeof(acceptData->clientAddress);
        acceptData->clientFd = accept(serverFd, (struct sockaddr *)&acceptData->clientAddress, &clientSocketLength);

        pthread_create(&tid, NULL, thread_start, acceptData);
        pthread_join(tid, NULL);
        pthread_detach(tid);
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
