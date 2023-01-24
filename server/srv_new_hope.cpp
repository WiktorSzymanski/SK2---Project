#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_CLIENTS 100
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int server_fd, client_fd, max_fd, fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    fd_set readfds;
    char buffer[BUF_SIZE];
    int n;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Bind socket to address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    listen(server_fd, MAX_CLIENTS);

    // Initialize the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    max_fd = server_fd;

    while (1) {
        // Wait for activity on the file descriptor set
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            exit(1);
        }

        // Iterate through the file descriptor set
        for (fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &readfds)) {
                if (fd == server_fd) {
                    // Accept a new connection
                    client_len = sizeof(client_addr);
                    client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
                    if (client_fd < 0) {
                        perror("Error accepting connection");
                        exit(1);
                    }
                    // Add the new client to the file descriptor set
                    FD_SET(client_fd, &readfds);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                } else {
                    // Handle data from a connected client
                    n = recv(fd, buffer, BUF_SIZE, 0);
                      if (n < 0) {
                        perror("Error receiving data");
                        exit(1);
                    } else if (n == 0) {
                        // Client disconnected
                        close(fd);
                        FD_CLR(fd, &readfds);
                        printf("Client disconnected\n");
                    } else {
                        // Send data back to the client
                        send(fd, buffer, n, 0);
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}

