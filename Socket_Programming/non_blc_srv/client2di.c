#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    struct sockaddr_in saddr;
    int fd,ret_val;

    //Creating TCP socket . . .
    fd= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(fd==-1){
        fprintf(stderr,"socket failed\n");
        return -1;
    }
    printf("Created a socket with fd: %d\n" ,fd);

    //Initializing server address structure . . .
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9002);
    saddr.sin_addr.s_addr=INADDR_ANY;

    //Connecting to TCP socket . . .
    ret_val=connect(fd,(struct sockaddr *)&saddr,sizeof(struct sockaddr_in));
    if(ret_val==-1){
        fprintf(stderr,"COnnect failed\n");
        close(fd);
        return -1;
    }
    printf("Connected\n");

    char data_buf[20];
    do
    {
        for(int i=1; i<=20;i++){
            char num[256];
            snprintf(num,sizeof(num),"%d",i);
            send(fd, num, sizeof(num), 0);

            //Recieving data . . .
            char server_response[256];
            recv(fd, &server_response, sizeof(server_response), 0);

            //printing server's response . . .
            printf("Server says: %s\n", server_response);

        }
        break; 
    } while (1);

    close(fd);
    return 0;
    
}