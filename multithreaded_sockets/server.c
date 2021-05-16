#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

// Employee data structure
typedef struct {
    char ID[7];
    char name[10];
    double salary;
} employee_info;

// Psuedo employee database
employee_info ei[10] = {
    { "abc000", "name0", 10000.0 },
    { "abc001", "name1", 10001.1 },
    { "abc002", "name2", 10002.2 },
    { "abc003", "name3", 10003.3 },
    { "abc004", "name4", 10004.4 },
    { "abc005", "name5", 10005.5 },
    { "abc006", "name6", 10006.6 },
    { "abc007", "name7", 10007.7 },
    { "abc008", "name8", 10008.8 },
    { "abc009", "name9", 10009.9 },
};

// Detached thread routine that handles all interaction with client connections
void *worker(void* args) {
    pthread_t worker_id = pthread_self();
    printf("WORKER [%lu] >> Connection handler thread created and detached.\n", worker_id);

    int connection_socket = (int) *((int *) args);

    char* buffer = (char *) malloc(256);
    if(buffer == NULL) {
        printf("WORKER [%lu] >> Failed to allocate memory for buffer.\n", worker_id);
        exit(EXIT_FAILURE);
    }

    int i;
    for(;;) {
        if(read(connection_socket, buffer, 256) <= 0) {
            printf("WORKER [%lu] >> Failed to receive data from client.\n", worker_id);
        }
        printf("WORKER [%lu] >> [RECEIVED] %s\n", worker_id, buffer);
        if(strcmp(buffer, "STOP") == 0) break;
        buffer += 10;

        int found = 0;
        for(i = 0; i < 10; i++) {
            employee_info data = ei[i];
            if(strcmp(buffer, data.ID) == 0) {
                found = 1;
                char salary_str[32];
                sprintf(salary_str, "%lf", data.salary);
                char response_buffer[256] = "";
                strcat(response_buffer, data.ID);
                strcat(response_buffer, "|");
                strcat(response_buffer, data.name);
                strcat(response_buffer, "|");
                strcat(response_buffer, salary_str);
                int num_bytes_sent = sizeof(response_buffer);
                if(write(connection_socket, response_buffer, num_bytes_sent) != num_bytes_sent) {
                    printf("WORKER [%lu] >> Failed to sent error message to client.\n", worker_id);
                    continue;
                }
                printf("WORKER [%lu] >> Sent query response to client.\n", worker_id);
                break;
            }
        }
        if(!found) {
            char error_buffer[6] = "ERROR";
            if(write(connection_socket, error_buffer, 6) != 6) {
                printf("WORKER [%lu] >> Failed to sent error message to client.\n", worker_id);
                continue;
            }
            printf("WORKER [%lu] >> Sent error message to client.\n", worker_id);
        }
    }

    printf("WORKER [%lu] >> Goodbye\n", worker_id);
    close(connection_socket);
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    // Server structures for address and port
    struct sockaddr_in sad;
    int port;

    // Client structure for address and length
    struct sockaddr_in cad;
    unsigned int alen;

    // Socket descriptors
    int welcomeSocket, connectionSocket;

    // Convert port argument to int
    if(argc > 1) port = atoi(argv[1]);
    else {
        fprintf(stderr,"Usage: %s port-number\n",argv[0]);
        exit(1);
    }

    // Create socket for server to continually listen on
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(welcomeSocket < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    printf("SERVER >> Server socket successfully created.\n");

    // Bind server socket to a local address
    memset((char *)&sad,0,sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY;
    sad.sin_port = htons((u_short)port);
    if(bind(welcomeSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"bind failed\n");
        exit(1);
    }

    // Set server socket connection queue limit
    if(listen(welcomeSocket, 10) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }

    printf("SERVER >> Server socket successfully bound to local address.\n");

    // Main loop - server continually listens for connections and passes of to a new worker thread
    for(;;) {
        printf("\nSERVER >> Waiting for connection...\n");
        alen = sizeof(cad);
        if ((connectionSocket = accept(welcomeSocket, (struct sockaddr *)&cad, &alen)) < 0) {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }
        printf("SERVER >> [CONNECTION] Sending to worker thread...\n");

        pthread_t worker_id;
        if(pthread_create(&worker_id, NULL, worker, (void *) &connectionSocket)) {
            fprintf(stderr, "Failed to create worker thread.\n");
            exit(EXIT_FAILURE);
        }

        if(pthread_detach(worker_id)) {
            fprintf(stderr, "Failed to detach worker thread.\n");
            exit(EXIT_FAILURE);
        }
    }
}