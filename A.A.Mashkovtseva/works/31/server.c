#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>

#define S_PATH "./socket"
#define BUFF 1024
#define MAX_CLIENTS 100

int main() {
    int server_sock, client_sock;
    char buffer[BUFF];
    struct sockaddr_un addr;
    fd_set readfds;
    int max_fd;
    int clients[MAX_CLIENTS] = {0};
    int n_clients = 0;
    
    unlink(S_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, S_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_sock);
        return 1;
    }

    printf("Server has been successfully created.\nServer is listening to %s\n", S_PATH);

    if (listen(server_sock, 5) == -1) {
        perror("listen");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    printf("Waiting for clients\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        max_fd = server_sock;

        for (int i = 0; i < n_clients; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > max_fd) max_fd = clients[i];
            }
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(server_sock, &readfds)) {
            client_sock = accept(server_sock, NULL, NULL);
            if (client_sock != -1) {
                if (n_clients < MAX_CLIENTS) {
                    clients[n_clients++] = client_sock;
                    printf("Client connected (fd=%d)\n", client_sock);
                } else {
                    printf("Too many clients, rejecting fd=%d\n", client_sock);
                    close(client_sock);
                }
            } else {
                perror("accept");
            }
        }

        for (int i = 0; i < n_clients; i++) {
            if (clients[i] > 0 && FD_ISSET(clients[i], &readfds)) {
                int bytes = read(clients[i], buffer, BUFF - 1);
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    for (int j = 0; j < bytes; j++) {
                        buffer[j] = toupper((unsigned char)buffer[j]);
                    }
                    printf("%s\n", buffer);
                } else if (bytes == 0) {
                    printf("Client (fd=%d) disconnected\n", clients[i]);
                    close(clients[i]);
                    clients[i] = -1;
                } else {
                    perror("read");
                    close(clients[i]);
                    clients[i] = -1;
                }
            }
        }

        for (int i = 0; i < n_clients; ) {
            if (clients[i] == -1) {
                close(clients[i]);
                for (int j = i; j < n_clients - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                n_clients--;
            } else {
                i++;
            }
        }
    }

    for (int i = 0; i < n_clients; i++) {
        if (clients[i] > 0) close(clients[i]);
    }
    close(server_sock);
    unlink(S_PATH);
    printf("Server closed.\n");

    return 0;
}
