#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define MAX_LEN 1024



char username[100];

// Thread function to receive messages from the server
DWORD WINAPI receiveMessages(LPVOID socketDesc) {
    SOCKET sock = *(SOCKET*)socketDesc;
    char buffer[MAX_LEN];
    int bytes;
    char userjoinMsg[100];
    

    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';

        printf("\n%s\n", buffer);
        printf("[%s]: ", username);
        fflush(stdout);
        memset(buffer, 0, sizeof(buffer));
    }

    return 0;
}




int main() {
    char message[MAX_LEN], fullMessage[MAX_LEN + 100];
    SOCKET sd;
    struct sockaddr_in server;
    WSADATA wsa;

    // Get username
    printf("Enter your Username:\t");
    scanf("%s", username);
    getchar();  // clear newline left by scanf

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == INVALID_SOCKET) {
        printf("Could not create socket. Error: %d\n", WSAGetLastError());
        return 1;
    }

    // Server details
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Remote server 20.197.12.181
    
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to server
    if (connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("❌ Connection failed.\n");
        return 1;
    }

    printf("✅ Connected to the server. Start chatting!\n");

    send(sd,username,strlen(username),0);

    // Start the receive thread
    CreateThread(NULL, 0, receiveMessages, &sd, 0, NULL);

    // Main loop to send messages
    while (1) {
        printf("[%s]: ", username);
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // remove newline

        snprintf(fullMessage, sizeof(fullMessage), "[%s]: %s", username, message);
        send(sd, fullMessage, strlen(fullMessage), 0);
    }

    // Cleanup
    closesocket(sd);
    WSACleanup();

    return 0;
}
