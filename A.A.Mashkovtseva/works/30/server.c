#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define S_PATH "./socket"
#define BUFF 1024

int main() {
    int server_sock, client_sock;
    char buffer[BUFF];
    struct sockaddr_un addr;
    
    unlink(S_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1){
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

    if (listen(server_sock, 1) == -1) {
        perror("listen");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    printf("Waiting for a client\n");

    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("accept");
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    printf("Successful connection!\n");

    int bytes = read(client_sock, buffer, BUFF);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        for (int i = 0; i < bytes; i++){
            buffer[i] = toupper((unsigned char)buffer[i]);
        }
        printf("%s\n", buffer);
    } else {
        perror("read");
        close(client_sock);
        close(server_sock);
        unlink(S_PATH);
        return 1;
    }

    printf("Disconnection!\n");
    close(client_sock);
    close(server_sock);
    unlink(S_PATH);
    printf("Server was turned off\n");

    return 0;
}