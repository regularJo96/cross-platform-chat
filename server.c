#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define IP "192.168.1.23"
#define FLUSH fflush(stdout)

void* handle(void*);
int isExitFlag(char[]);
void send_to_all_clients(int socket, int connections[], int count);

struct connection_info{
    int client_sock;
    int* conns;
    int* conns_count;
};

int main(int argc, char const* argv[]){
    int connections[10];
    int index=0;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct connection_info pArgs[10]; 
    struct sockaddr_in server_addr;

    pthread_t threads[10];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8913);
    // works if you know the server address, but gotta be a more dynamic/generic way to find 
    // perhaps by hostname. Was able to find connections on a specific host in a C# client
    server_addr.sin_addr.s_addr = inet_addr(IP);

    bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    listen(server_socket, 10);

    while(1){
        
        connections[index] = accept(server_socket, NULL, NULL);
        
        printf("client connected.\n");
        fflush( stdout );

        pArgs[index].client_sock = connections[index];
        pArgs[index].conns = connections;
        pArgs[index].conns_count = &index;
        index++;
 
        //pass the address of the proper indexed struct from the array
        if(pthread_create(&threads[index-1], NULL, handle, (void*)&pArgs[index-1]) < 0){
            printf("error creating thread");
            FLUSH;
        }
    }

    close(server_socket);

    return 0;
}

void* handle(void* pStruct){
    // a holder for the client socket number, the address of the first connection in array
    // (could convert to linked list later) 
    // a holer for the actual number of connections, as the array has fixed size.
    int client_socket = ((struct connection_info *)pStruct)->client_sock;
    int* first_connection = ((struct connection_info *)pStruct)->conns;
    int* count = ((struct connection_info *)pStruct)->conns_count;

    printf("client socket:%d\n",client_socket);
    fflush( stdout );
    int connected = 1;
    char msg_recv[255];

    while(connected){
        bzero(msg_recv, 255);
        printf("waiting...\n");
        fflush(stdout);
        if(recv(client_socket, msg_recv, 255, 0) < 0){
            printf("error receiving from socket. Closing connection.\n");
            fflush( stdout );
            close(client_socket);
            return NULL;
        }
        
        //if the frst character in the message is a period, it might be the exit flag. Check.
        if(msg_recv[0] == '.'){
            connected = isExitFlag(msg_recv);
        }

        printf("message received from client on socket %d.\nMessage Content: %s\n", client_socket, msg_recv);
        fflush( stdout );
        // go through all connections and send message
        int * iterator;
        int i=0;
        printf("this is socket: %d\n", client_socket);
        fflush( stdout );
        for(iterator = first_connection; i<*count; iterator++){
            // printf("checking socket %d\n",*iterator);
            // fflush( stdout );

            // if exit message from client, send ack to client
            if(connected==0){
                printf("sending exit ack to client on socket %d\n", client_socket);
                fflush(stdout);

                if(send(client_socket, msg_recv, 255, 0) < 0){
                    printf("error sending to socket. Closing connection.\n");
                    fflush( stdout );
                    close(*iterator);
                    return NULL;
                }
            }   
            
            
            if(*iterator != client_socket){
                printf("sending to conected client on socket %d.\n",*iterator);
                fflush(stdout);

                //if exit message from client, send message to other clients that
                // their buddy disconnected.
                if(connected==0){
                    if(send(*iterator, "client disconnected.<EOL>", 255, 0) < 0){
                        printf("error sending to socket. Closing connection.\n");
                        fflush( stdout );
                        close(*iterator);
                        return NULL;
                    }

                //else send contents of message to conected clients
                } else{
                    if(send(*iterator, msg_recv, 255, 0) < 0){
                        printf("error sending to socket. Closing connection.\n");
                        fflush( stdout );
                        close(*iterator);
                        return NULL;
                    }
                }
                    
            }
            i++;
        }
    }
    printf("Client disconnected.\n");
    fflush( stdout );
    close(client_socket);
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