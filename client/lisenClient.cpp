#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

void request(char *addres, int port) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    char temp[100];

    struct hostent* addrent;
    struct sockaddr_in addr;

    addrent = gethostbyname(addres);

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);

    // char * buf = new char [strlen(to) + strlen(message) + 2];
    
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != -1) {

        char buf[512];
        
        std::cout << "\nMode: ";
        std::cin >> temp;

        strcpy(buf, temp);
        strcat(buf, ";");

        memset(temp, 0, 100);

        std::cout << "\nTo: ";
        std::cin >> temp;

        strcat(buf, temp);
        strcat(buf, ";");
        memset(temp, 0, 100);

        std::cout << "\nMessage: ";
        std::cin >> temp;

        strcat(buf, temp);
        strcat(buf, ";");
        memset(temp, 0, 100);

    
        write(fd, buf, strlen(buf));
        memset(buf, 0, 512); 
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);

            
        memset(buf, 0, 512); 
        while(1) {
            memset(buf, 0, 512); 
            read(fd, buf, sizeof(buf));
            printf("%s\n", buf);
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Brak agrumentow: DOMENA i PORT\n");
        return 0;
    }

    // while(1) {
        request(argv[1], atoi(argv[2]));
    // }

    return 0;
}