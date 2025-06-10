#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <Winsock2.h>

#define MAX_CLIENTS 10
SOCKET user[100];
int clientcount = 0;
CRITICAL_SECTION cs;

DWORD WINAPI MessageHandle(LPVOID clientSocket)
{
    SOCKET cd = *(SOCKET *)clientSocket;
    char rcv[1000];
    int bytes;

    // Notify users of another user join
    char username[50];
    char joinMsg[50];
    recv(cd,username,sizeof(username),0);
    snprintf(joinMsg,sizeof(joinMsg),"%s Just Joined the chat!!",username);

    EnterCriticalSection(&cs);
    for (int i = 0; i < clientcount; i++)
    {
        if (user[i] != cd)
        {
            send(user[i],joinMsg,strlen(joinMsg),0);
        }
        
    }
    LeaveCriticalSection(&cs);
    





    // Broadcasting message to all client
    while ((bytes = recv(cd, rcv, sizeof(rcv) - 1, 0)) > 0)
    {
        rcv[bytes] = '\0';

        EnterCriticalSection(&cs);
        for (int i = 0; i < clientcount; i++) // 1
        {
            if (user[i] != cd) // user[0] = 120
            {
                send(user[i], rcv, bytes, 0);
            }
        }
        LeaveCriticalSection(&cs);
    }

    // Removing the user after Disconnected
    EnterCriticalSection(&cs);
    for (int i = 0; i < clientcount; i++) // 1
    {
        if (user[i] == cd) // user[0] = 120
        {
            for (int j = i; i < clientcount - 1; i++)
            // 2 5 7 9
            {
                user[j] = user[j + 1];
            }
            clientcount--;
        }
    }

    LeaveCriticalSection(&cs);
    closesocket(cd);
    return 0;
}

int main()
{

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    SOCKET sd, cd;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd >= 0)
    {
        printf("Server Started!!\n");
    }

    struct sockaddr_in server, client;

    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = ADDR_ANY;

    int bindcheck = bind(sd, (struct sockaddr *)&server, sizeof(server));
    if (bindcheck == SOCKET_ERROR)
    {
        printf("%s", WSAGetLastError);
        closesocket(sd);
        WSACleanup();
        return 1;
    }

    listen(sd, 5);

    int c = sizeof(struct sockaddr_in);
    InitializeCriticalSection(&cs);

    while (1)
    {
        cd = accept(sd, (struct sockaddr *)&client, &c);
        printf("Client Connection Successfull!!\n");
        EnterCriticalSection(&cs);
        if (clientcount < MAX_CLIENTS)
        {
            user[clientcount++] = cd; // user[5] = 123
            SOCKET *newClient = malloc(sizeof(SOCKET));
            *newClient = cd;
            CreateThread(NULL, 0, MessageHandle, newClient, 0, NULL);
        }
        else
        {
            printf("Too many clients.\n");
            closesocket(cd);
        }
        LeaveCriticalSection(&cs);
    }

    DeleteCriticalSection(&cs);
    closesocket(cd);
    closesocket(sd);
    WSACleanup();
}



// testing the repo contro