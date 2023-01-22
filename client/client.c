#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void request(char *addres, int port, char *index) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);

    struct hostent* addrent;
    struct sockaddr_in addr;

    addrent = gethostbyname(addres);

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);

    char buf[512];

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != -1) {
        write(fd, index, strlen(index));
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Brak agrumentow: DOMENA, PORT i NUMER INDEXU\n");
        return 0;
    }

    // while(1) {
        request(argv[1], atoi(argv[2]), argv[3]);
    // }

    return 0;
}