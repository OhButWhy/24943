#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define S_PATH "./socket"

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_un addr;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"Text\"\n", argv[0]);
        return 1;
    }

    const char *message = argv[1];
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, S_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(client_socket);
        return 1;
    }

    printf("Client was connected with server\n");

    if (write(client_socket, message, strlen(message)) == -1) {
        perror("message");
        return 1;
    }

    printf("Message was delivered\n");
    close(client_socket);
    printf("Connection closed\n");

    return 0;
}