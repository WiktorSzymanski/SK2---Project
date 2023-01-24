#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>


#define BUFFER_SIZE 512
#define BACKLOG_SIZE 5
#define SERVER_PORT 1234

struct clientln {
    int cfd;
    struct sockaddr_in caddr;
};

void* cthread(void* arg) {
    char* buffer = new char[BUFFER_SIZE];
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";
    struct clientln* c = (struct clientln*)arg;

    printf("\e[32m[CONNECTED]\e[0m: %s\n\e[33m[MESSAGE]\e[0m: ", inet_ntoa((struct in_addr)c -> caddr.sin_addr));

    while(1) {
        read(c->cfd, buffer, BUFFER_SIZE);

        printf("%s\n\n", buffer);

        write(c->cfd, buffer, strlen(buffer));

        memset(buffer, 0, 512);
    }
}

int main(int argc, char **argv) {
    socklen_t clientSocketLength;
    pthread_t tid;
    struct sockaddr_in serverAddress, clientAddress;
    
    int serverFd = socket(PF_INET, SOCK_STREAM, 0),
        cfd, 
        on = 1;

    // Mówi systemowi operacyjnemu aby uwalniał port po wyłączeniu serwera
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // wszystkie adresy ip w systemie
    serverAddress.sin_port = htons(SERVER_PORT);

    int isPortAlreadyTaken = -1 == bind(serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if(isPortAlreadyTaken) {
        printf("Couldn't open socket.\n");
        return EXIT_FAILURE;
    }

    listen(serverFd, BACKLOG_SIZE);

    while(1) {
        struct clientln* c = new clientln();

        clientSocketLength = sizeof(c->caddr);
        std::cout << "Accept next"<< std::endl;
        c -> cfd = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);
        std::cout << c->cfd << std::endl;
        pthread_create(&tid, NULL, cthread, c);
        pthread_detach(tid);
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
