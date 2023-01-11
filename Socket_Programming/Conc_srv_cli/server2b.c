#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>


struct thread_args {
    int client_socket;
    struct sockaddr_in client_address;
};

void * func(void *_args);

int fact(int ans){  
  if (ans == 0)  
    return 1;  
  else  
    return (ans * fact(ans-1));  
}  

int main(){

    // char server_message[256]="YOU'VE REACHED SERVER . . . !!";

    
    //Creating socket . . .
    int server_socket,c;
    server_socket= socket(AF_INET,SOCK_STREAM,0);

    //Define server address . . .
    struct sockaddr_in server_address, client_address;
    server_address.sin_family= AF_INET;
    server_address.sin_port =htons(9002);
    server_address.sin_addr.s_addr= INADDR_ANY;

    //Binding socket to an IP nd port . . .
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    
    //Listen for requests . . . 
    listen(server_socket,10);

    pid_t childpid;
    while(1){

        c=sizeof(struct sockaddr_in);
        //Accepting client request . . .
        int client_socket =accept(server_socket, (struct sockaddr *)&client_address,(socklen_t*)&c);

        pthread_t th1;
        struct thread_args *args = malloc (sizeof (struct thread_args));
        args->client_socket = client_socket;
        args->client_address = client_address;
        // pthread_create(&th1,NULL,func,args);
        if((childpid=fork())==0){
            close(server_socket);

            while(1){
                func(args);
            }
        }

    }

    //Closing the server . . .
    close(server_socket);
    return 0;
}

void * func(void *_args){
    struct thread_args *args = (struct thread_args *) _args;
    // args->client_socket;
    // args->client_address;

    //Opening and Creating a file . . .
    
    //file pointer
    FILE *fp = NULL;
    //create the file
    printf("Connected to client %d\n",args->client_socket);
    fp = fopen("data.txt", "w");
    if(fp == NULL){
        printf("Error in creating the file\n");
        exit(1);
    }

    for(int i=1; i<=20;i++){
        
        //Receiving data from client . . .
        char client_response[256];
        recv(args->client_socket, &client_response, sizeof(client_response), 0);

        //Sending the requested data . . .
        int factorial;
        sscanf(client_response, "%d", &factorial);  //Converting client response to int factorial . . .
        factorial = fact(factorial);                //Clculating the factorial . . .
        char server_response[256];                  
        snprintf(server_response,sizeof(server_response),"%d",factorial); // Storing server response in buffer . . .
        send(args->client_socket,server_response,sizeof(server_response),0);    // Sending server response to client . . .

        //Writing data in the file . . .
        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&args->client_address;
        struct in_addr ipAddr = pV4Addr->sin_addr;

        char *client_ip = inet_ntoa(args->client_address.sin_addr);
        int client_port = ntohs(args->client_address.sin_port);
        fprintf(fp,"%s",client_ip);
        fprintf(fp,", %d",client_port);
        fprintf(fp,", %d\n",factorial);
        // fprintf ("%s\n", args->client_socket);
        // printf ("%d\n", args->client_socket);

    }


    // free (args);
    //Closing the file . . .
    fclose(fp);
    pthread_exit (NULL);
}