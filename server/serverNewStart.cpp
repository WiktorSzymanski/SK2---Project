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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#define BUFFER_SIZE 512
#define BACKLOG_SIZE 10
#define SERVER_PORT 1234


int main(int argc, char **argv) {
    socklen_t clientSocketLength;
    struct sockaddr_in serverAddress, clientAddress;
    std::vector<std::string> messages;


    std::map<std::string,int> clientsFds; 

    
    int serverFd = socket(PF_INET, SOCK_STREAM, 0),
        clientFd, 
        on = 1,
        fdMax, fd_count, rc;

    char buffer[BUFFER_SIZE];
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";

    static struct timeval timeout;
    fd_set filesDescriptors, readFilesDescriptors, writeFilesDescriptors, resp1, resp2;

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

    FD_ZERO(&filesDescriptors);
    FD_ZERO(&readFilesDescriptors);
    FD_ZERO(&writeFilesDescriptors);

    fdMax = serverFd;

    while(1) {
        printf("Server started\n");
        readFilesDescriptors = filesDescriptors;
        FD_SET(serverFd, &readFilesDescriptors);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        printf("Imma before select\n");
        rc = select(fdMax+1, &readFilesDescriptors, &writeFilesDescriptors, (fd_set*)0, &timeout);
        printf("Imma after select\n");
        if (rc == 0) {
            printf("timed out\n");
            continue;
        }
        fd_count = rc;
        if (FD_ISSET(serverFd, &readFilesDescriptors)) {+
            printf("Imma in connection\n");
            fd_count -= 1;
            clientSocketLength = sizeof(clientAddress);
            clientFd = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);
            printf("Connection: %s\n",
                inet_ntoa((struct in_addr)clientAddress.sin_addr));
            FD_SET(clientFd, &filesDescriptors);
            if (clientFd > fdMax) fdMax = clientFd;
        }
        for (int i = serverFd + 1; i <= fdMax && fd_count > 0; i++) {
            if (FD_ISSET(i, &writeFilesDescriptors)) {
                fd_count -= 1;

                while(!messages.empty()) {
                    std::string mess = messages.front();
                    messages.erase(messages.begin());
                    char * buf = new char [mess.length() + 1];
                    strcpy (buf, mess.c_str());

                    write(i, buf, strlen(buf));
                    std::cout << buf << " was send" << std::endl;
                }
            

                // close(i);
                FD_CLR(i, &writeFilesDescriptors);
                FD_SET(i, &readFilesDescriptors);
                if (i == fdMax)
                    while (fdMax > serverFd && !FD_ISSET(fdMax, &filesDescriptors) && !FD_ISSET(fdMax, &writeFilesDescriptors))
                        fdMax -= 1;
            }
            else if (FD_ISSET(i, &readFilesDescriptors)) {
                fd_count -= 1;
                FD_CLR(i, &filesDescriptors);
                
                read(i, buffer, BUFFER_SIZE);

                printf("SUIII %s\n\n", buffer);

                std::stringstream sstream;
                sstream << buffer;

                std::string segment;

                while(std::getline(sstream, segment, '\n')) {
                    messages.push_back(segment);
                }

                FD_SET(i, &writeFilesDescriptors);
                FD_CLR(i, &readFilesDescriptors);
            }
        }
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
