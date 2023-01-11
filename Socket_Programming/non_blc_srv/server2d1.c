#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
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
    // initializing file discriptor set
    fd_set readfds;

    int r_socket;
    if ((r_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error while making a socket");
        exit(1);
    }

    // creating struct
    struct sockaddr_in selectServer, concurrentClient;
    memset(&selectServer, '\0', sizeof(selectServer));
    selectServer.sin_family = AF_INET;                     // match the socket() call
    selectServer.sin_addr.s_addr = INADDR_ANY; // bind to any local address
    selectServer.sin_port = htons(9002);

    // bind socket and server
    int r_bind;
    if ((r_bind = bind(r_socket, (const struct sockaddr *)&selectServer, sizeof(selectServer))) == -1)
    {
        perror("Error while binding socket and server");
        exit(1);
    }

    // listening to the client request
    int r_listening;
    if ((r_listening = listen(r_socket, 10)) == -1)
    {
        perror("Error while to listening to the client request");
        exit(1);
    }

    int maxClient = 10;
    int clientSocket[maxClient];

    /* Initializing Client Socket to Zero */
    int c = 0;
    while (c < maxClient)
    {
        clientSocket[c] = 0;
        c++;
    }

    clock_t time;
    int i = 0;
    int n = 0;
    int count = 0;
    while (1)
    {

        // clearing the set
        FD_ZERO(&readfds);
        FD_SET(r_socket, &readfds);

        int maxVal = r_socket;

        /* Adding Child Socket to the Set. */
        for (int i = 0; i < maxClient; i++)
        {
            if (clientSocket[i] > 0)
            {
                FD_SET(clientSocket[i], &readfds);
            }
            if (clientSocket[i] > maxVal)
            {
                maxVal = clientSocket[i];
            }
        }

        // select System Call
        int r_select = select(maxVal + 1, &readfds, NULL, NULL, NULL);
        if (r_select == -1)
        {
            perror("select() system call Failed!\n");
            exit(EXIT_FAILURE);
        }

        int clientSize = sizeof(concurrentClient);
        if (FD_ISSET(r_socket, &readfds))
        {

            printf("------------------------------------------\n");
            printf("\nWaiting for request...\n");

            int r_accepted;
            if ((r_accepted = accept(r_socket, (struct sockaddr *)&concurrentClient, &clientSize)) == -1)
            {
                perror("Error while accepting the client request");
                exit(1);
            }
            n++;
            if (n == 1)
            {
                time = clock();
            }

            // Adding New Socket //
            int k = 0;
            while (k < maxClient)
            {
                if (clientSocket[k] == 0)
                {
                    clientSocket[k] = r_accepted;
                    break;
                }
                i++;
            }
        }

        int message;
        long fact;

        for (int i = 0; i < maxClient; i++)
        {
            int fdiscriptor = clientSocket[i];
            if (FD_ISSET(fdiscriptor, &readfds))
            {
                FILE *fileptr;
                fileptr = fopen("concurrentClient_SelectServer.txt", "a");
                if (fileptr == NULL)
                {
                    printf("Error in opening the file for appending\n");
                    exit(1);
                }

                memset(&message, 0, sizeof(int));
                memset(&fact, 0, sizeof(long));

                int r_recieved = recv(fdiscriptor, &message, sizeof(int), 0);
                if (r_recieved == 0)
                {
                    close(fdiscriptor);
                    clientSocket[i] = 0;
                    break;
                }

                printf("Message from client: %d\n", message);

                fact = factorial(message);

                int size = sizeof(concurrentClient);
                getpeername(fdiscriptor, (struct sockaddr *)&concurrentClient, (socklen_t *)&size);
                char *ip_address = inet_ntoa(concurrentClient.sin_addr);
                int port = ntohs(concurrentClient.sin_port);

                fprintf(fileptr, "IP Address: %size, Port: %d | Number: %d , Factorial: %ld\n", ip_address, port, message, fact);

                send(fdiscriptor, &fact, sizeof(long), 0);

                printf("Message sent to client: %ld\n", fact);

                fclose(fileptr);

                count++;
                if (count == 200)
                {
                    time = clock() - time;
                    double time_taken = ((double)time) / CLOCKS_PER_SEC;
                    printf("Time taken to request 10 clients: %.15lf\n", time_taken);
                }
            }
        }
        i++;
    }

    close(r_socket);
}