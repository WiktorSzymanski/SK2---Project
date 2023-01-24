#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void request(char *addres, int port, char *to, char *message) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);

    struct hostent* addrent;
    struct sockaddr_in addr;

    addrent = gethostbyname(addres);

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);

    // char * buf = new char [strlen(to) + strlen(message) + 2];

    char buf[512];

    char readBuf[100];

    strcpy(buf, to);
    strcat(buf, "\n");
    strcat(buf, message);
    strcat(buf, "\n");

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != -1) {
        write(fd, buf, strlen(buf));
        read(fd, readBuf, sizeof(readBuf));
        printf("%s\n", readBuf);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Brak agrumentow: DOMENA, PORT, USER_NAME i MESSAGE\n");
        return 0;
    }

    // while(1) {
        request(argv[1], atoi(argv[2]), argv[3], argv[4]);
    // }

    return 0;
}