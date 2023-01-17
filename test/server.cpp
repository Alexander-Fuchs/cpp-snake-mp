// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>
#define PORT 65010

int counter = 0;

void *worker(void* new_socket)
{
    int socket = *(int*)new_socket;
    char buffer[1024] = { 0 };
    const char* hello = "Hello from server";
    while (read(socket, buffer, 1024)) {
        counter++;
        printf("%s\n", buffer);
        send(socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        printf("%d\n", counter);
    }
}

int main(int argc, char const* argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    const char* hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) > 0) {
        printf("Left main thread\n");
        pthread_t workerthread;
        int ret = pthread_create(&workerthread, NULL, worker, &new_socket);
        pthread_join(workerthread, NULL);
    }

    // close(new_socket);
    // closing the listening socket
    // shutdown(server_fd, SHUT_RDWR);

    printf("Left main thread\n");

    return 0;
}
