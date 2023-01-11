#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  /*Contains structure that we can define a few fields on
                           so that we know the address and the port of the particular
                           remote socket that we're gonna connect to*/
#include <arpa/inet.h>
#include <pthread.h>


void * func(void *_args);

int main(){
    int num,i=0;
  
    // Input the number of clients . . .
    printf("Enter the number of clients: ");
    scanf("%d", &num);

    //Creating thread . . .
    pthread_t th[num];
    while(i<num){
        if( pthread_create(&th[i], NULL, func, NULL) != 0 ){
            printf("Failed to create thread\n");
        }
        i++;
    }
    sleep(20);
    i = 0;
    while(i< num){
        pthread_join(th[i++],NULL);
        printf("%d:\n",i);
    }
    return 0;
}

void * func(void *_args){
    /*Creating a socket(Domain || Type       || protocol)*/ 
    int sockfd=  socket(AF_INET,  SOCK_STREAM,  0);

    //specify an address for the socket
    struct sockaddr_in server_addr; //desclaring the structure for this address
    server_addr.sin_family= AF_INET; //family of the address
    server_addr.sin_port= htons(9002);//We could pass int but data format is slightly different from the structure
                          //so we need to use a conversion function but since we've included all the 
                          //appropriate headers so its already here. The conversion function that's gonna put 
                          //the integer port that we pass in the right network byte order is "htons".
                          //This function takes care of converting port number we pass it into appropriate data format
                          //so out structure can identify the right port no. and we know where we're connecting to.
    server_addr.sin_addr.s_addr= INADDR_ANY;

    //Connecting to server . . .
    int connection_status= connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    //A little error handelling . . .
    if(connection_status==-1) printf("ERROR, COULDN'T CONNECT TO SERVER \n");

    //For loop to send 20 requests
    for(int i=1; i<=20;i++){
        char num[256];
        snprintf(num,sizeof(num),"%d",i);
        send(sockfd, num, sizeof(num), 0);

        //Recieving data . . .
        char server_response[256];
        recv(sockfd, &server_response, sizeof(server_response), 0);

        //printing server's response . . .
        printf("Server says: %s\n", server_response);

    }

    //Closing socket . . .
    close(sockfd);
}