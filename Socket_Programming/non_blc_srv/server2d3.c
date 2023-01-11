#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>

long factorial(int n)
{
    long f = 1;
    int k = 1;
    while (k <= n)
    {
        f = f * k;
        k++;
    }
    return f;
}

int main(){


    int r_socket;
    if((r_socket = socket(AF_INET, SOCK_STREAM,0)) == -1){
        perror("Error while making a socket");
        exit(1);
    }

    // creating struct
    struct sockaddr_in epollServer, concurrentClient;
    memset(&epollServer, '\0', sizeof(epollServer)); 
    epollServer.sin_family = AF_INET; // match the socket() call
    epollServer.sin_addr.s_addr = INADDR_ANY; // bind to any local address
    epollServer.sin_port = htons(9002); 

    // bind socket and server
    int r_bind;
    if((r_bind = bind(r_socket,(const struct sockaddr*)&epollServer,sizeof(epollServer))) == -1){
        perror("Error while binding socket and server");
        exit(1);
    }

    // listening to the client request
    int r_listening;
    if((r_listening = listen(r_socket,11)) == -1){
        perror("Error while listening to the client request");
        exit(1);
    }

    struct epoll_event epoll,fdiscriptor[11];
    memset(fdiscriptor,0,sizeof(fdiscriptor));
    
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("Error while creating epoll");
        exit(EXIT_FAILURE);
    }

    epoll.data.fd = r_socket;
    epoll.events = EPOLLIN;
    
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, r_socket, &epoll) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    clock_t time;
    int n = 0;
    int count = 0;

    while(1){

        int size = epoll_wait(epollfd, fdiscriptor, 11, -1);
        if (size == -1) {
            perror("Error while waiting for epoll");
            exit(EXIT_FAILURE);
        }

        int l = sizeof(concurrentClient);
        int curr = size;
        for(int i=0;i<size;i++){
            if((fdiscriptor[i].events & EPOLLIN ) != EPOLLIN) continue;

            int current_fdiscriptor = fdiscriptor[i].data.fd;
            if(current_fdiscriptor == r_socket){
                int r_accepted;

                // accepting incoming clients
                printf("Waiting for request\n");
                printf("----------------------------------\n");
                r_accepted = accept(r_socket,(struct sockaddr *) &concurrentClient, &l);
                if(r_accepted == -1){
                    perror("Error while accepting incoming clients");
                    exit(1);
                }

                n++;
                if(n == 1){
                    time = clock();
                }

                epoll.data.fd = r_accepted;
                epoll.events = EPOLLIN;

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, r_accepted,&epoll) == -1) {
                    perror("Error in epoll_ctl");
                    exit(EXIT_FAILURE);
                }
            }
            else{

                int message;
                long fact;

                memset(&message,0,sizeof(int));
                memset(&fact,0,sizeof(long));

                int r_recieved = recv(current_fdiscriptor,&message,sizeof(int),0);
                
                if(r_recieved < 0){
                    perror("Error while recieving file");
                    close(r_socket);
                    continue;
                }
                else if(r_recieved == 0){
                    close(current_fdiscriptor);
                    fdiscriptor[i].data.fd = -1*current_fdiscriptor;
                    break;
                }

                printf("Message from client: %d\n",message);

                fact = factorial(message);

                getpeername(current_fdiscriptor , (struct sockaddr*)&concurrentClient , (socklen_t*)&l);
                char *ip_address = inet_ntoa(concurrentClient.sin_addr);
                int port = ntohs(concurrentClient.sin_port);

                FILE *fileptr;
                fileptr = fopen("concurrentClient_epollServer.txt","a");
                if(fileptr==NULL){
                    printf("Error in opening the file for appending\n");
                    exit(1);
                }

                fprintf(fileptr, "IP Address: %s, Port: %d | Number: %d , Factorial: %ld\n",ip_address, port,message ,fact);

                send(current_fdiscriptor,&fact,sizeof(long),0);

                printf("Message sent to client: %ld\n",fact);

                count++;
                if(count==200){
                    time = clock()-time;
                    double time_taken = ((double)time)/CLOCKS_PER_SEC;
                    printf("Time taken to request 10 clients : %.15lf\n",time_taken);

                }
                fclose(fileptr);

            }
        }

    }
}