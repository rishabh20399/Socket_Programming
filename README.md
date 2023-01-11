# Socket_Programming
https://github.com/rishabh20399/Socket_Programming/blob/main/SocketProgramming.pdf

A client server socket program in C. The program should has the following features. 

  1. Client program: 
  
    a. Sequential client: Each client generates 20 requests; Each request consists of an integer, i.e., ith request is an integer “i” 
       ranging from 1 to 20; display the received response on the terminal; close the connection after 20 requests.
    
    b. Concurrent client: For designs 2(b), 2(c), and 2(d), the client program is a multithreaded program that generates concurrent 
       requests. For example, if we want 10 concurrent client programs, the client code should run 10 parallel threads. 
    
  2. Server program: opens a new file when the first client connects; for each client request: compute the factorial of the received number; store the result along          with client-id (IP address, port number) in the file and also send the result back to the client; close the file when all the client connections is closed. 
  
    Following are the designs of the server programs- 
    a. Sequential server program.
    b. Concurrent server program with multiple processes (using fork system call) 
    c. Concurrent server program with multiple threads (using pthreads) 
    d. Non-blocking server that can manage total 10 clients 
    
      i. Using select() system call 
      ii. Using poll() system call  
      iii. Using epoll API

