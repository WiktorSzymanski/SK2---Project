#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>

#define BUFFER_SIZE 512
#define BACKLOG_SIZE 10
#define SERVER_PORT 1234


int main(int argc, char **argv) {
    socklen_t clientSocketLength;
    struct sockaddr_in serverAddress, clientAddress;
    
    int serverFd = socket(PF_INET, SOCK_STREAM, 0),
        clientFd, 
        on = 1,
        fdMax, fda, rc;

    char buffer[BUFFER_SIZE];
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";

    static struct timeval timeout;
    fd_set mask, rmask, wmask, resp1, resp2;

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

    FD_ZERO(&mask);
    FD_ZERO(&rmask);
    FD_ZERO(&wmask);

    fdMax = serverFd;

    while(1) {
        printf("Server started\n");
        rmask = mask;
        FD_SET(serverFd, &rmask);
        timeout.tv_sec = 60 * 5;
        timeout.tv_usec = 0;
        rc = select(fdMax+1, &rmask, &wmask, (fd_set*)0, &timeout);

        if (rc == 0) {
            printf("timed out\n");
            continue;
        }
        fda = rc;
        if (FD_ISSET(serverFd, &rmask)) {
            fda -= 1;
            clientSocketLength = sizeof(clientAddress);
            clientFd = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);
            printf("Connection: %s\n",
                inet_ntoa((struct in_addr)clientAddress.sin_addr));
            FD_SET(clientFd, &mask);
            if (clientFd > fdMax) fdMax = clientFd;
        }
        for (int i = serverFd + 1; i <= fdMax && fda > 0; i++) {
            if (FD_ISSET(i, &wmask)) {
                fda -= 1;
                
                if(FD_ISSET(i, &resp1)) {
                    char *r = "Odpowiedz 1\n";
                    int len = strlen(r);
                    int sentCount = 0;
                   do{
                   	sentCount = write(i,r, strlen(r));
                   	r += sentCount;
                   } while(sentCount < len);
                    FD_CLR(i, &resp1);
                    
                } else if(FD_ISSET(i, &resp2)) {
                
                    char *r = "Odpowiedz 2\n";
                    int len = strlen(r);
                    int sentCount = 0;
                   do{
                   	int temp = write(i,r+sentCount,strlen(r));
                   	r += temp;
                   	sentCount += temp;
                   } while(sentCount < len);
                    FD_CLR(i, &resp1);
                } else {
                    write(i, errorMessage, BUFFER_SIZE);
                }

                close(i);
                FD_CLR(i, &wmask);
                if (i == fdMax)
                    while (fdMax > serverFd && !FD_ISSET(fdMax, &mask) && !FD_ISSET(fdMax, &wmask))
                        fdMax -= 1;
            }
            // kolejność inna or sth
            else if (FD_ISSET(i, &rmask)) {
                fda -= 1;
                FD_CLR(i, &mask);
                
                read(i, buffer, BUFFER_SIZE);

                printf("SUIII %s\n\n", buffer);

                if(strncmp(buffer, "148165", 6) == 0) {
                    FD_SET(i, &resp1);
                } else if(strncmp(buffer, "148084", 6) == 0) {
                    FD_SET(i, &resp2);
                }

                FD_SET(i, &wmask);
                FD_CLR(i, &rmask);
            }
        }
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
