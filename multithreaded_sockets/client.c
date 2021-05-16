#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

void clear_stdin(FILE* fp) {
    int c;
    while ( (c = fgetc(fp)) != EOF && c != '\n');
}

int main(int argc, char** argv) {
    // Structures to hold server data
    struct sockaddr_in sad;
    int clientSocket;
    struct hostent *ptrh;

    // Host name and port
    char *host;
    int port;

    // Check for valid argument usage
    if(argc != 3) {
        fprintf(stderr, "Usage: %s server-name port-number\n", argv[0]);
        exit(1);
    }

    // Extract host name from arguments
    host = argv[1];

    // Extract and convert port number from arguments
    port = atoi(argv[2]);

    // Create client socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    printf("CLIENT >> Client socket successfully created.\n");

    // Connect client socket to server socket
    memset((char *)&sad,0,sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_port = htons((u_short)port);
    ptrh = gethostbyname(host);
    if(((char *) ptrh) == NULL) {
        fprintf(stderr,"invalid host: %s\n", host);
        exit(1);
    }
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    if(connect(clientSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"connect failed\n");
        exit(1);
    }
    printf("CLIENT >> Client socket successfully connected to local server socket.\n");


    // Initial welcome message

    // Main loop - continually ask and handle menu options
    int option;
    char em_ID[32];
    for(;;) {
        printf("\nCLIENT >> Choose an option below:\n");
        printf("       >> [ 1 ] Get salary of an employee.\n");
        printf("       >> [ 2 ] Exit.\n");
        printf("       >> Enter an option: ");
        option = -1;
        scanf("%d", &option);
        if(option < 1 || option > 2) {
            printf("CLIENT >> [ERROR] Invalid answer. Try again...\n");
            clear_stdin(stdin);
            continue;
        }
        if(option == 2) break;
        if(option == 1) {
            printf("\nCLIENT >> Enter an employee ID: ");
            scanf("%s", em_ID);
            char request[64] = "GETSALARY ";
            strcat(request, em_ID);
            int num_bytes_sent = sizeof(request);
            if(write(clientSocket, request, num_bytes_sent) != num_bytes_sent) {
                printf("CLIENT >> [ERROR] Failed to send request to server.\n");
                continue;
            }
            printf("CLIENT >> Sent request [%s] to server...\n", request);

            char response[256];
            if(read(clientSocket, response, 256) <= 0) {
                printf("CLIENT >> [ERROR] Failed to receive correct response from server.\n");
                continue;
            }
            if(strcmp(response, "ERROR") == 0) {
                printf("CLIENT >> [ERROR] Server claims employee ID is invalid.\n");
                continue;
            }

            printf("CLIENT >> Response from server:\n");
            char *save_token;
            char *token;
            token = strtok_r(response, "|", &save_token);
            int token_i = 0;
            while(token != NULL) {
                printf("       >> ");
                switch(token_i) {
                    case 0:
                        printf("ID: %s\n", token);
                        break;
                    case 1:
                        printf("Name: %s\n", token);
                        break;
                    case 2:
                        printf("Salary: %s\n", token);
                        break;
                    default:
                        printf("Invalid token: %s\n", token);
                        break;
                }
                token = strtok_r(NULL, "|", &save_token);
                token_i++;
            }
        }
    }

    printf("CLIENT >> Exiting...\n");
    char exit_request[5] = "STOP";
    if(write(clientSocket, exit_request, 5) != 5) {
        printf("CLIENT >> [ERROR] Failed to send exit notification to server.\n");
        exit(EXIT_FAILURE);
    }
    close(clientSocket);
    printf("CLIENT >> Goodbye.\n");

    return 0;
}