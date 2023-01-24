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
#include <map>

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
};

struct ThreadData {
    User user;
    std::vector<User>* users;
    pthread_mutex_t* users_mutex;
    std::map<std::string, std::vector<Message>>* messagesData;
};

void sendMessage(int mode, Message* message, User* user) {
    std::string stringMessage = std::to_string(mode) + ";" + message->from + ";" + message->message + ";";
    write(user->userFileDescriptor, stringMessage.c_str(), strlen(stringMessage.c_str()));
};

void* cthread(void* arg) {
    char* buffer = new char[BUFFER_SIZE];
    char errorMessage[BUFFER_SIZE] = "\e[31mERROR\e[0m";
    struct ThreadData* tData = (struct ThreadData*)arg;
    int readRet;

    printf("\e[32m[CONNECTED]\e[0m: %s\n\e[33m[MESSAGE]\e[0m: ", inet_ntoa((struct in_addr)tData -> user.userAddr.sin_addr));

    while(1) {
        readRet = read(tData -> user.userFileDescriptor, buffer, BUFFER_SIZE);

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
                std::cout << "Loging in: " << to << "\n";
                tData->user.username = to;
                bool uniqName = true;
                Message message;
                message.from = "SYS";

                for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
                    if (tData->user.username.compare(it->username) == 0) {
                        std::cout << "Error - this username already exists\n";
                        message.message = "Error - username " + tData->user.username + " already exists!";
                        sendMessage(1, &message, &tData -> user);
                        uniqName = false;
                    }
                }

                if (!uniqName) {
                    break;
                }
                std::cout << "Added " << tData->user.username << " to users list\n";
                tData->users->push_back(tData->user);

                message.message = "Logged in as " + tData->user.username;
                std::cout << "Send SYS message" << std::endl;
                sendMessage(1, &message, &tData -> user);
                for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
                        if (tData->user.username.compare(it->first) == 0) {
                            for (auto m : it->second) {
                                sendMessage(2, &m, &tData -> user);
                            }
                        }
                        tData->messagesData->erase(it);
                        break;
                    }
            } else if (mode.compare("2") == 0) {
                bool userIsActive = false;
                std::cout << "Sending data\nFrom: " << tData -> user.username << "\nTo: " << to << std::endl;

                Message mess;

                mess.from = tData->user.username;
                mess.message = message;

                for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
                    if (to.compare(it->username) == 0) {
                        sendMessage(2, &mess, it.base());
                        userIsActive = true;
                        break;
                    }
                }

                if (!userIsActive) {
                    bool newInThis = true;
                    // std::cout << "Non dilivered messages:" << std::endl;
                    for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
                        // std::cout << it->first << std::endl;
                        if (to.compare(it->first) == 0) {
                            it->second.push_back(mess);
                            newInThis = false;
                        }
                    }

                    if (newInThis) {
                        std::vector<Message> userMessages;
                        userMessages.push_back(mess);
                        tData->messagesData->insert(std::make_pair(to, userMessages));
                    }
                }


            } else if (mode.compare("9") == 0) {
                std::cout << "User " << tData -> user.username << " is disconnecting" << std::endl;
                for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
                    if (tData->user.username.compare(it->username) == 0) {
                        Message message;
                        message.from = "SYS";
                        message.message = "Disconnecting...";
                        sendMessage(9, &message, &tData -> user);
                        tData->users->erase(it);
                        break;
                    }
                }
                close(tData->user.userFileDescriptor);
                break;
            }

            // write(tData -> user.userFileDescriptor, buffer, strlen(buffer));


        memset(buffer, 0, 512);
        std::cout << "\tCurrent User List:\n";
        for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
            std::cout << "\t\t" << it->username << std::endl;
        }



        std::cout << "\tCurrent Non dilivered messages:\n";
        for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
                std::cout << "\t\t" << it->first << std::endl;

                for (auto m : it->second) {
                    std::cout << "\t\t\t\t" << m.from << " : " << m.message << std::endl; 
                }
            }
        }
    printf("Connection ended\n");
}

int main(int argc, char **argv) {
    std::vector<User> users;
    std::map<std::string, std::vector<Message>>* messegesData = new std::map<std::string, std::vector<Message>>;

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
        struct User client;
        struct ThreadData* tData = new ThreadData();

        std::cout << "Accept next"<< std::endl;
        clientSocketLength = sizeof(client.userAddr);
        client.userFileDescriptor = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);
        tData -> users = &users;
        tData -> users_mutex = &users_mutex;
        tData -> user = client;
        tData -> messagesData = messegesData;
        std::cout << client.userFileDescriptor << std::endl;
        pthread_create(&tid, NULL, cthread, tData);
        pthread_detach(tid);
    }

    close(serverFd);

    return EXIT_SUCCESS;
}
