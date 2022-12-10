#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define IP "192.168.1.23"
#define FLUSH fflush(stdout)

int isExitFlag(char[]);
void* send_message(void*);
void* recv_message(void*);

int main(int argc, char const* argv[]){
    char name[16];
    int connected = 1;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;

    pthread_t send;
    pthread_t recv;

    int *pClient = &client_socket;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8913);
    // works if you know the server address, but gotta be a more dynamic/generic way to find 
    // perhaps by hostname. Was able to find connections on a specific host in a C# client
    server_addr.sin_addr.s_addr = inet_addr(IP);

    printf("Enter a name to use while chatting: ");
    FLUSH;
    scanf("%15s", name);

    int connectionStatus = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(connectionStatus < 0){
        printf("there was an error trying to connect to the server.\n");
        FLUSH;
    }

    while(connected){

        if(connectionStatus < 0){
            puts("Error occurred with connection.");
        }
        else{
            pthread_create(&send, NULL, send_message, (void*)pClient);
            pthread_create(&recv, NULL, recv_message, (void*)pClient);
        }
    }
    printf("closing client connection.\n");
    close(client_socket);
    
    return 0;

}

void* send_message(void * pClient){
    char msg[255];
    bzero(msg, 255);
    scanf("%255s", msg);
    strcat(msg,"<EOL>");
    printf("socket: %d", *((int*)pClient));
    FLUSH;
    send(*((int*)pClient), msg, 255, 0);
    printf("sent message.\n");
    FLUSH;
    return NULL;
}

void* recv_message(void * pClient){
    char msg[255];
    recv(*((int*)pClient), msg, 255, 0);
    return NULL;
}

int isExitFlag(char str[]){
    char exitFlag[] = ".exit<EOL>";
    for(int i=0;i<sizeof(exitFlag);i++){
        if(exitFlag[i] != str[i]){
            return 1;
        }
    }
    return 0;
}