#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
char password[50];
#define MAX_LEN 1024
struct userinformation
{

    char username[50];
    char email[60];
    char password[50];
};
struct userinformation user1;

// Function

// Input function

void takeinput(char ch[50])
{
    fgets(ch, 50, stdin);
    size_t len = strlen(ch);
    if (len > 0 && ch[len - 1] == '\n')
    {
        ch[len - 1] = '\0';
    }
}

// generateUsername function
void generateUsername(char email[50], char username[50])
{

    for (int i = 0; i < strlen(email); i++)
    {
        if (email[i] == '@')
        {
            break;
        }
        else
        {
            username[i] = email[i];
        }
    }
}

// Verifyuser function

int verifyUser()
{

    FILE *file;
    int attempt = 3;
    char u[50], p[50];
    char fulldata[100], fulldata2[100];

    

    while (attempt)
    {

        int found = 0;
        printf("Enter Your username: ");
        takeinput(user1.username);
        printf("Enter Your password: ");
        takeinput(user1.password);
        file = fopen("userdata.dat", "r");

        while (fscanf(file, "%s %s", u, p) == 2)
        {
            if (strcmp(u, user1.username) == 0 && strcmp(p, user1.password) == 0)
            {
                
                found = 1;
                break;
                
            }
        }
       fclose(file);


        if (found)
        {
            printf("Successfully Logged in!\n");
            return 1;
        }
        
        else
        {
            attempt--;
            printf("Your credential doesnt matched\n");
            if (attempt == 0)
            {
                return 0;
            }
        }
        // snprintf(fulldata2, sizeof(fulldata2), "%s %s", u, p);
    }
}



// Authentication Function

int auth()
{
    int choice;
    int check;
    int attempt = 3;
    FILE *file;
    printf("1.Signup\n");
    printf("2.Login\n");
    printf("3.Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    getchar();

    switch (choice)
    {
    case 1:
        while (1)
        {
            printf("Enter your Email: ");
            scanf("%s", user1.email);
            getchar();
            generateUsername(user1.email, user1.username);
            // Check if username already exists
            file = fopen("userdata.dat", "r");
            if (file != NULL)
            {
                char existingUser[50];
                int userExists = 0;

                while (fscanf(file, "%s", existingUser) == 1)
                {
                    if (strcmp(existingUser, user1.username) == 0)
                    {
                        userExists = 1;
                        break;
                    }
                }
                fclose(file);

                if (userExists)
                {
                    printf("Error: Username '%s' is already taken. Please use a different email.\n", user1.username);

                    continue;
                }
                else
                {
                    break;
                }
            }
        }

        while (attempt > 0)
        {

            printf("\nEnter your Password: ");
            scanf("%s", user1.password);
            getchar();
            printf("\nPlease Confirm Your Password: ");
            scanf("%s", password);
            if (!strcmp(user1.password, password))
            {
                printf("\nPassword matched!!\n");

                break;
            }
            else
            {
                printf("Password doesnt matched, try again!\n");
                attempt--;
                printf("You have %d Attempt left\n", attempt);

                Beep(750, 400);
            }
        }

        if (attempt == 0)
        {
            printf("Good bye!!");
            return 0;
        }

        printf("\nYour Username is %s\n", user1.username);
        file = fopen("userdata.dat", "a+");
        if (file != NULL)
        {
            fprintf(file, "%s %s\n", user1.username, user1.password);
            fclose(file); // Important!
            printf("User Registration Successful, Your Username is %s\n", user1.username);
        }
        else
        {
            printf("Error opening userdata.dat for writing.\n");
        }

        break;

    case 2:
        check = verifyUser();
        if (check == 0)
        {
            verifyUser();
        }
        else if (check == 1)
        {
            break;
        }

    default:

        return 0;
    }
}

// Thread function to receive messages from the server
DWORD WINAPI receiveMessages(LPVOID socketDesc)
{

    SOCKET sock = *(SOCKET *)socketDesc;
    char buffer[MAX_LEN];
    int bytes;
    char userjoinMsg[100];

    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes] = '\0';

        printf("\n%s\n", buffer);
        printf("[%s]: ", user1.username);
        fflush(stdout);
        memset(buffer, 0, sizeof(buffer));
    }

    return 0;
}

int main()
{

    system("color 0A");

    char message[MAX_LEN], fullMessage[MAX_LEN + 100];
    SOCKET sd;
    struct sockaddr_in server;
    WSADATA wsa;

    // Authentication
    int goodbye = auth();

    if (goodbye == 0)
    {
        return 0;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == INVALID_SOCKET)
    {
        printf("Could not create socket. Error: %d\n", WSAGetLastError());
        return 1;
    }

    // Server details
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Remote server 20.197.12.181

    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to server
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed.\n");
        return 1;
    }

    printf("Connected to the server. Start chatting!\n");

    send(sd, user1.username, strlen(user1.username), 0);

    // Start the receive thread
    CreateThread(NULL, 0, receiveMessages, &sd, 0, NULL);

    // Main loop to send messages
    // fgets(message, sizeof(message), stdin);
    while (1)
    {
        printf("[%s]: ", user1.username);
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // remove newline

        snprintf(fullMessage, sizeof(fullMessage), "[%s]: %s", user1.username, message);
        send(sd, fullMessage, strlen(fullMessage), 0);
    }

    // Cleanup
    closesocket(sd);
    WSACleanup();

    return 0;
}
