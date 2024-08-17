#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h> 
#include <time.h>

#define PORT 8080

int valid_name(char* name);
void* receive_message(void* arg);
void* send_message(void* arg);
void clear_screen();

int network_socket;

int main() {
    pthread_t send_thread, receive_thread;
    char name[100];

    struct sockaddr_in server_socket;

    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_socket.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(PORT);

    if (connect(network_socket, (struct sockaddr*)&server_socket, sizeof(server_socket)) < 0) {
        perror("Connection failed");
        close(network_socket);
        return -1;
    }

    do {
        printf("Insert a valid name:\n");
        scanf("%99s", name);
    } while (!valid_name(name));

    if (send(network_socket, name, strlen(name) + 1, 0) < 0) {
        perror("Send name failed");
        close(network_socket);
        return -1;
    }

    clear_screen();

    if (pthread_create(&send_thread, NULL, send_message, NULL) != 0) {
        perror("Failed to create send thread");
        close(network_socket);
        return -1;
    }

    if (pthread_create(&receive_thread, NULL, receive_message, NULL) != 0) {
        perror("Failed to create receive thread");
        close(network_socket);
        return -1;
    }

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);
    
    close(network_socket);

    return 0;
}

int valid_name(char* name) {
    return (name != NULL && strlen(name) > 2 && *name >= 'A' && *name <= 'Z');
}

void* receive_message(void* arg) {
    char buffer[256];
    bool valid_connection = true;

    while (valid_connection) {
        int bytes = recv(network_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            if (strlen(buffer) > 0) {
                printf("\033[31m[Server]: \033[0m%s", buffer);
            }
        } else if (bytes == 0) {
            printf("\033[33mServer disconnected\033[0m\n");
            valid_connection = false;
        } else if (bytes < 0) {
            perror("\033[33mReceive failed\033[0m\n");
            valid_connection = false;
        }
    }
    close(network_socket);
    return NULL;
}



void* send_message(void* arg) {
    char buffer2[256];
    bool valid_connection = true;

    while (valid_connection) {
        if (fgets(buffer2, 255, stdin) != NULL) {
            if (buffer2[0] != '\n') {
                if (send(network_socket, buffer2, strlen(buffer2), 0) < 0) {
                    perror("Send failed");
                    valid_connection = false;
                }
                buffer2[0] = '\n';
            }
        } else {
            perror("Input read failed");
            valid_connection = false;
        }
    }

    close(network_socket);
    return NULL;
}



void clear_screen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}
