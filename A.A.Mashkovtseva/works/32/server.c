#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#define S_PATH "./socket"
#define BUFF 1024
#define MAX_CLIENTS 100

int main() {
    int server_sock, client_sock;
    char buffer[BUFF];
    struct sockaddr_un addr;
    int clients[MAX_CLIENTS];
    int n_clients = 0;
    int flags;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

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

    if (listen(server_sock, 5) == -1) {
        perror("listen");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    flags = fcntl(server_sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }
    if (fcntl(server_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    printf("Server has been successfully created.\nServer is listening to %s\n", S_PATH);
    printf("Waiting for clients (non-blocking mode)\n");

    while (1) {
        while (1) {
            client_sock = accept(server_sock, NULL, NULL);
            if (client_sock == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    perror("accept");
                    break;
                }
            }

            flags = fcntl(client_sock, F_GETFL, 0);
            if (flags != -1) {
                fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
            }

            if (n_clients < MAX_CLIENTS) {
                clients[n_clients++] = client_sock;
                printf("Client connected (fd=%d)\n", client_sock);
            } else {
                printf("Too many clients, rejecting fd=%d\n", client_sock);
                close(client_sock);
            }
        }

        for (int i = 0; i < n_clients; i++) {
            int fd = clients[i];
            if (fd < 0) continue;

            while (1) {
                int bytes = read(fd, buffer, BUFF - 1);
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    for (int j = 0; j < bytes; j++) {
                        buffer[j] = toupper((unsigned char)buffer[j]);
                    }
                    printf("%s\n", buffer);
                } else if (bytes == 0) {
                    printf("Client disconnected (fd=%d)\n", fd);
                    close(fd);
                    clients[i] = -1;
                    break;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    } else if (errno == EINTR) {
                        continue;
                    } else {
                        perror("read");
                        close(fd);
                        clients[i] = -1;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < n_clients; ) {
            if (clients[i] == -1) {
                for (int j = i; j < n_clients - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                clients[n_clients - 1] = -1;
                n_clients--;
            } else {
                i++;
            }
        }

        usleep(10000);
    }

    return 0;
}