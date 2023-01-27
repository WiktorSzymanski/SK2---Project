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
#include <set>
#include <cstring>

#define BUFFER_SIZE 512
#define BACKLOG_SIZE 10
#define SERVER_PORT 1234
#define ERROR "\e[31m[ERROR]: \e[0m"
#define ALLERT "\e[31m[ALLERT]: \e[0m"
#define INFO "\e[33m[INFO]: \e[0m"

pthread_mutex_t USERS_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MESSAGES_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t FRIENDS_MUTEX = PTHREAD_MUTEX_INITIALIZER;

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
    std::map<std::string, std::vector<Message>>* messagesData;
    std::map<std::string, std::set<std::string>>* friends;
};

void sendMessage(int mode, Message* message, User* user) {
    std::string stringMessage = std::to_string(mode) + ";" + message->from + ";" + message->message + "\n";
    write(user->userFileDescriptor, stringMessage.c_str(), strlen(stringMessage.c_str()));
};

void printLogSeparator(std::string section) {
    if (section != "") section = " " + section + " ";
    
    int length = 60;
    int left_padding = 15;
    int right_padding = length - section.length() - left_padding;


    std::cout << "\n" << std::string(left_padding, '=') <<  section << std::string(right_padding, '=') << std::endl;
}

void printActiveStatusLoggs(ThreadData* tData) {

    printLogSeparator("CURRENT ACTIVE USERS");
    pthread_mutex_lock(&USERS_MUTEX);
    for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
        std::cout << "\t" << it->username << std::endl;
    }
    pthread_mutex_unlock(&USERS_MUTEX);
    printLogSeparator("");

    printLogSeparator("CURRENT NON-DILIVERED MESSAGES");
    pthread_mutex_lock(&MESSAGES_MUTEX);
    for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
            std::cout << "\t" << it->first << std::endl;

            for (auto m : it->second) {
                std::cout << "\t\t" << m.from << " : " << m.message << std::endl; 
            }
        }
    pthread_mutex_unlock(&MESSAGES_MUTEX);
    printLogSeparator("");

    printLogSeparator("CURRENT FRIENDS LISTS");
    pthread_mutex_lock(&FRIENDS_MUTEX);
    for (std::map<std::string, std::set<std::string>>::iterator it = tData->friends->begin(); it != tData->friends->end(); ++it) {
            std::cout << "\t" << it->first << std::endl;

            for (auto f : it->second) {
                std::cout << "\t\t" << f << std::endl; 
            }
        }
    pthread_mutex_unlock(&FRIENDS_MUTEX);
    printLogSeparator("");
}

bool disconnectProcess(ThreadData* tData) {
    std::cout << INFO << "User " << tData -> user.username << " is disconnecting" << std::endl;

    pthread_mutex_lock(&USERS_MUTEX);
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
    pthread_mutex_unlock(&USERS_MUTEX);

    close(tData->user.userFileDescriptor);
    return true;
}

void friendsProcess(ThreadData* tData, std::string to){
    std::cout << INFO << "Adding friend to user: " << tData->user.username  << "\n";


    if (to.compare("") != 0) {
        bool newInThis = true;
        pthread_mutex_lock(&FRIENDS_MUTEX);
        for (std::map<std::string, std::set<std::string>>::iterator it = tData->friends->begin(); it != tData->friends->end(); ++it) {
            if (tData->user.username.compare(it->first) == 0) {
                it->second.insert(to);
                newInThis = false;
            }
        }

        if (newInThis) {
            std::set<std::string> friends;
            friends.insert(to);
            tData->friends->insert(std::make_pair(tData->user.username, friends));
        }
        pthread_mutex_unlock(&FRIENDS_MUTEX);
    }

    Message message;
    message.from = "SYS";
    message.message = "";

    pthread_mutex_lock(&FRIENDS_MUTEX);
    for (std::map<std::string, std::set<std::string>>::iterator it = tData->friends->begin(); it != tData->friends->end(); ++it) {
        if (tData->user.username.compare(it->first) == 0) {
            for (auto f : it->second) {
                if (message.message != "") {
                    message.message += ":";
                }
                message.message += f;
            }
            break;
        }
    }
    pthread_mutex_unlock(&FRIENDS_MUTEX);

    sendMessage(4, &message, &tData -> user);
}

void currentUsersProcess(ThreadData* tData, std::string to) {
    std::cout << INFO << "Sending current users list to: " << tData->user.username  << "\n";
    Message message;
    message.from = "SYS";
    message.message = "";

    pthread_mutex_lock(&USERS_MUTEX);
    for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
        if (tData->user.username.compare(it->username) == 0) {
            continue;
        }

        if (message.message != "") {
            message.message += ":";
        }
        message.message += it->username;
    }
    pthread_mutex_unlock(&USERS_MUTEX);

    sendMessage(1, &message, &tData -> user);
}

void messageProcess(ThreadData* tData, std::string to, std::string message) {
    bool userIsActive = false;
    std::cout << INFO << "Sending data\n\tFrom: " << tData -> user.username << "\n\tTo: " << to << std::endl;

    Message mess;

    mess.from = tData->user.username;
    mess.message = message;

    pthread_mutex_lock(&USERS_MUTEX);
    for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
        if (to.compare(it->username) == 0) {
            sendMessage(2, &mess, it.base());
            userIsActive = true;
            break;
        }
    }
    pthread_mutex_unlock(&USERS_MUTEX);

    if (!userIsActive) {
        bool newInThis = true;
        pthread_mutex_lock(&MESSAGES_MUTEX);
        for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
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
        pthread_mutex_unlock(&MESSAGES_MUTEX);
    }
}

bool loginUser(ThreadData* tData, std::string to) {
    Message message;
    message.from = "SYS";

    tData->user.username = to;
    std::cout << INFO << "Loging in: " << tData->user.username << "\n";

    pthread_mutex_lock(&USERS_MUTEX);
    for (std::vector<User>::iterator it = tData->users->begin(); it != tData->users->end(); ++it) {
        if (tData->user.username.compare(it->username) == 0) {
            message.message = "ERROR: username " + tData->user.username + " already exists!";
            sendMessage(9, &message, &tData -> user);
            std::cout << ERROR << tData->user.username << " username already exists\n";

            return false;
        }
    }
    std::cout << INFO << "Added " << tData->user.username << " to users list\n";
    tData->users->push_back(tData->user);
    pthread_mutex_unlock(&USERS_MUTEX);

    message.message = "Logged in as " + tData->user.username;
    std::cout << INFO << "Send SYS message" << std::endl;
    sendMessage(1, &message, &tData -> user);
    pthread_mutex_lock(&MESSAGES_MUTEX);
    for (std::map<std::string, std::vector<Message>>::iterator it = tData->messagesData->begin(); it != tData->messagesData->end(); ++it) {
            if (tData->user.username.compare(it->first) == 0) {
                for (auto m : it->second) {
                    sendMessage(2, &m, &tData -> user);
                }
            }
            tData->messagesData->erase(it);
            break;
        }
    pthread_mutex_unlock(&MESSAGES_MUTEX);

    return true;
}

void* clientThread(void* arg) {
    struct ThreadData* tData = (struct ThreadData*)arg;
    char* buffer = new char[BUFFER_SIZE];

    std::cout << "\e[32m[CONNECTED]\e[0m: " << inet_ntoa((struct in_addr)tData -> user.userAddr.sin_addr) << std::endl;

    while(1) {
        int readRet = read(tData -> user.userFileDescriptor, buffer, BUFFER_SIZE);

        if (readRet == -1) {
            std::cout << ERROR << "reading error - braeaking loop!" << std::endl;
            break;
        } else if (readRet == 0) {
            continue;
        }
            std::stringstream sstream;
            std::string mode, to, message;

            sstream << buffer;

            std::getline(sstream, mode, ';');
            std::getline(sstream, to, ';');
            std::getline(sstream, message, ';');

            if (mode.compare("1") == 0) {
                if (!loginUser(tData, to)) break;
            } else if (mode.compare("2") == 0) {
                messageProcess(tData, to, message);
            } else if (mode.compare("3") == 0) {
                currentUsersProcess(tData, to);
            } else if (mode.compare("4") == 0) {
                friendsProcess(tData, to);
            } else if (mode.compare("9") == 0) {
                if (disconnectProcess(tData)) break;
            }

        memset(buffer, '\0', BUFFER_SIZE);
        printActiveStatusLoggs(tData);
        }

    std::cout << INFO << "User disconected" << std::endl;

    return nullptr;
}

int main(int argc, char **argv) {
    std::vector<User> mUsers;
    std::map<std::string, std::vector<Message>>* mMessagesData = new std::map<std::string, std::vector<Message>>;
    std::map<std::string, std::set<std::string>>* mFriendsData = new std::map<std::string, std::set<std::string>>;


    socklen_t clientSocketLength;
    pthread_t tid;
    struct sockaddr_in serverAddress, clientAddress;
    
    int serverFd = socket(PF_INET, SOCK_STREAM, 0), on = 1;

    // Mówi systemowi operacyjnemu aby uwalniał port po wyłączeniu serwera
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // wszystkie adresy ip w systemie
    serverAddress.sin_port = htons(SERVER_PORT);

    int isPortAlreadyTaken = -1 == bind(serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if(isPortAlreadyTaken) {
        std::cout << ERROR << "Couldn't open socket!" << std::endl;
        return EXIT_FAILURE;
    }

    listen(serverFd, BACKLOG_SIZE);

    while(1) {
        if (mUsers.size() >= BACKLOG_SIZE) {
            std::cout << ALLERT << "Max users reached" << std::endl;
            sleep(5);
            continue;
        }

        struct User client;
        struct ThreadData* tData = new ThreadData();

        std::cout << INFO << "Ready to accept..."<< std::endl;
        clientSocketLength = sizeof(client.userAddr);
        client.userFileDescriptor = accept(serverFd, (struct sockaddr*)&clientAddress, &clientSocketLength);

        tData -> users = &mUsers;
        tData -> messagesData = mMessagesData;
        tData -> friends = mFriendsData; 
        tData -> user = client;

        pthread_create(&tid, NULL, clientThread, tData);
        pthread_detach(tid);

        if (mUsers.size() + 1 >= BACKLOG_SIZE) {
            std::cout << ALLERT << "Max users reached" << std::endl;
            sleep(5);
            continue;
        }
    }
    
    std::cout << INFO << "Server shuting down..." << std::endl;
    close(serverFd);

    return EXIT_SUCCESS;
}
