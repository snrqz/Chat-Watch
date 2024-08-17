#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h> 
#include <string.h>
#include <time.h> 
#include <sys/select.h>

#define PORT 8080
#define TIMEOUT_SEC 10
#define MAX_TRIES 3

int server_socket, client_socket;
char name[100];

void clear_screen();
void* send_message(void* arg);
void* receive_message(void* arg);

int main() {
    pthread_t send_thread, receive_thread;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Network socket error");
        return -1;
    }

    struct sockaddr_in server_socket_addr, client_socket_addr;

    server_socket_addr.sin_family = AF_INET;
    server_socket_addr.sin_port = htons(PORT);
    server_socket_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_socket_addr, sizeof(server_socket_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 1) < 0) {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }

    printf("Waiting for a client connection . . .\n");

    socklen_t addr_size = sizeof(client_socket_addr);
    client_socket = accept(server_socket, (struct sockaddr*)&client_socket_addr, &addr_size);
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        return -1;
    }

    printf("A client has joined\n");

    clear_screen();

    if (recv(client_socket, name, sizeof(name), 0) < 0) {
        perror("Receive name failed");
        close(client_socket);
        close(server_socket);
        return -1;
    }
    printf("[%s] has joined the server\n", name);

    if (pthread_create(&send_thread, NULL, send_message, NULL) != 0) {
        perror("Failed to create send thread");
        close(client_socket);
        close(server_socket);
        return -1;
    }

    if (pthread_create(&receive_thread, NULL, receive_message, NULL) != 0) {
        perror("Failed to create receive thread");
        close(client_socket);
        close(server_socket);
        return -1;
    }

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(client_socket);
    close(server_socket);

    return 0;
}

void clear_screen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void* send_message(void* arg) {
    char buffer[256];
    bool valid_connection = true; 
    while (valid_connection) {
        if (fgets(buffer, 255, stdin) == NULL) {
            perror("Input read failed");
            valid_connection = false;
        }
        if (send(client_socket, buffer, sizeof(buffer), 0) < 0) {
            perror("Send failed");
            valid_connection = false;
        } 
    }
    close(client_socket); 
    return NULL;
}

void* receive_message(void* arg) {
    char buffer2[256];
    time_t start_time = time(NULL);
    time_t current_time;
    int ctr = 0;
    bool valid_connection = true;
    struct timeval timeout;
    fd_set read_fds;

    while (valid_connection) {
        current_time = time(NULL);

        if (ctr >= MAX_TRIES) {
            char msg[] = "\033[38;5;214mServer is quitting . . .\033[0m\n";
            send(client_socket, msg, strlen(msg), 0);
            close(client_socket);
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);

        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);

        if (activity == 0) {
            char msg[] = "\033[38;5;214mSystem is waiting for your response . . .\033[0m\n";
            send(client_socket, msg, strlen(msg), 0);
            start_time = current_time;
            ctr++;
        } else if (activity > 0 && FD_ISSET(client_socket, &read_fds)) {
            int bytes = recv(client_socket, buffer2, sizeof(buffer2) - 1, 0);

            if (bytes > 0) {
                buffer2[bytes] = '\0';
                if (strlen(buffer2) > 0) {
                    printf("\033[32m[%s]: \033[0m%s", name, buffer2);
                    start_time = current_time;
                    ctr = 0; 
                }
            } else if (bytes == 0) {
                printf("\033[33mClient disconnected\033[0m\n");
                valid_connection = false;
            } else if (bytes < 0) {
                perror("\033[33mReceive failed\033[0m\n");
                valid_connection = false;
            }
        }
    }
    close(client_socket);
    return NULL;
}
