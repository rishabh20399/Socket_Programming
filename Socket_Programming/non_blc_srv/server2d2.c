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
#include <sys/poll.h>
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

int main()
{

    int r_socket;
    if ((r_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error while making a socket");
        exit(1);
    }

    // creating struct
    struct sockaddr_in pollServer, concurrentClient;
    memset(&pollServer, '\0', sizeof(pollServer));
    pollServer.sin_family = AF_INET;                     // match the socket() call
    pollServer.sin_addr.s_addr = INADDR_ANY; // bind to any local address
    pollServer.sin_port = htons(9002);

    // bind socket and pollServer
    int r_bind;
    if ((r_bind = bind(r_socket, (const struct sockaddr *)&pollServer, sizeof(pollServer))) == -1)
    {
        perror("Error while binding socket and server");
        exit(1);
    }

    // listening to the client request
    int r_listening;
    if ((r_listening = listen(r_socket, 11)) == -1)
    {
        perror("Error while listening to the client request");
        exit(1);
    }

    struct pollfd fdiscriptor[11];
    memset(fdiscriptor, 0, sizeof(fdiscriptor));

    fdiscriptor[0].fd = r_socket;
    fdiscriptor[0].events = POLLIN;
    fdiscriptor[0].revents = 0;
    int size = 1;

    clock_t time;
    int n = 0;
    int count = 0;
    while (1)
    {

        int r_poll = poll(fdiscriptor, size, -1);
        if (r_poll < 0)
        {
            perror("poll() system call Failed!");
            break;
        }

        int clientSize = sizeof(concurrentClient);
        int current = size;
        for (int i = 0; i < current; i++)
        {
            if ((fdiscriptor[i].revents && POLLIN) != POLLIN)
                continue;

            int current_fdiscriptor = fdiscriptor[i].fd;
            if (current_fdiscriptor == r_socket)
            {
                int ret_acc;

                // accepting incoming clients
                printf("Waiting for request\n");
                printf("----------------------------------\n");
                ret_acc = accept(r_socket, (struct sockaddr *)&concurrentClient, &clientSize);
                if (ret_acc == -1)
                {
                    perror("Error while accepting incoming clients");
                    exit(1);
                }

                n++;
                if (n == 1)
                {
                    time = clock();
                }

                size++;
                fdiscriptor[size - 1].fd = ret_acc;
                fdiscriptor[size - 1].events = POLLIN;
                fdiscriptor[size - 1].revents = 0;
            }
            else
            {

                int message;
                long fact;

                memset(&message, 0, sizeof(int));
                memset(&fact, 0, sizeof(long));

                int r_recieved = recv(current_fdiscriptor, &message, sizeof(int), 0);

                if (r_recieved < 0)
                {
                    perror("Error while recieving file");
                    close(r_socket);
                    continue;
                }
                else if (r_recieved == 0)
                {
                    close(current_fdiscriptor);
                    fdiscriptor[i].fd = -1 * current_fdiscriptor;
                    break;
                }

                printf("Message from client: %d\n", message);

                fact = factorial(message);

                getpeername(current_fdiscriptor, (struct sockaddr *)&concurrentClient, (socklen_t *)&clientSize);
                char *ip_address = inet_ntoa(concurrentClient.sin_addr);
                int port = ntohs(concurrentClient.sin_port);

                FILE *fileptr;
                fileptr = fopen("concurrentClient_PollServer.txt", "a");
                if (fileptr == NULL)
                {
                    printf("Error in opening the file for appending\n");
                    exit(1);
                }

                fprintf(fileptr, "IP Address: %s, Port: %d | Number : %d , Factorial: %ld\n", ip_address, port, message, fact);

                send(current_fdiscriptor, &fact, sizeof(long), 0);
                printf("Message sent to client: %ld\n", fact);

                count++;
                if (count == 200)
                {
                    time = clock() - time;
                    double time_taken = ((double)time) / CLOCKS_PER_SEC;
                    printf("Time taken to request 10 clients: %.15lf\n", time_taken);
                }

                fclose(fileptr);
            }
        }
    }
}