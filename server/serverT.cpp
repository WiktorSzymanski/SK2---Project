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
#include <vector>
#include <sstream>

#define BUFFER_SIZE 512
#define BACKLOG_SIZE 5
#define SERVER_PORT 1234

pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

struct Message {
    std::string from;
    std::string message;
};

struct User {
    std::string username;
    int userFileDescriptor;
    struct sockaddr_in userAddr;
    std::vector<Message> messages;
    std::vector<User>* users;
    pthread_mutex_t* users_mutex;
};


void* cthread(void* arg) {
    char* buffer = new char[BUFFER_SIZE];
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";
    struct User* client = (struct User*)arg;
    int readRet;

    printf("\e[32m[CONNECTED]\e[0m: %s\n\e[33m[MESSAGE]\e[0m: ", inet_ntoa((struct in_addr)client -> userAddr.sin_addr));

    while(1) {
        readRet = read(client->userFileDescriptor, buffer, BUFFER_SIZE);

        if (readRet == -1) {
            printf("Reading error - braeaking loop\n");
            break;
        } else if (readRet == 0) {
            continue;
        }
            // printf(buffer);

            std::stringstream sstream;
            sstream << buffer;

            std::string mode, to, message;

            std::getline(sstream, mode, ';');
            std::getline(sstream, to, ';');
            std::getline(sstream, message, ';');

            std::cout << "\nMode: " << mode << "\nTo: " << to << "\nMessage: " << message << "\n\n";

            if (mode.compare("1") == 0) {
                std::cout << "Loging in: " << to;

                for (int i = 0; i < client->users.size(); i++)
            }

            write(client->userFileDescriptor, buffer, strlen(buffer));


            memset(buffer, 0, 512);
    }
    printf("Connection ended\n");
}

int main(int argc, char **argv) {
    std::vector<User> users;

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
        struct User* client = new User();

        clientSocketLength = sizeof(client->userAddr);
        std::cout << "Accept next"<< std::endl;
        client -> userFileDescriptor = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);
        client -> users = &users;
        client -> users_mutex = &users_mutex;
        std::cout << client -> userFileDescriptor << std::endl;
        pthread_create(&tid, NULL, cthread, client);
        pthread_detach(tid);
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
