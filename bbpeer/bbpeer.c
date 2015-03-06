/*
 * bbpeer.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */


#include <stdio.h> //header files from book
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h> // for threading
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>

#include "bbpeer.h"
#include "bbWriter.h"

ServerInfo serverInfo;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: main

    Accepts a file in argv[1]. The main/user thread operates the menu.

    @param argc         --  number of args
    @param argv         --  arg vector
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, const char* argv[])
{
    pthread_t networkThread; //thread for token and IO

    if(argc != 4)
    {
        printf("Usage: %s <filename><serverIP><serverPort>\n", argv[0]);
        return -1;
    }

    int                sockfd;
    struct sockaddr_in servaddr;
    char               response[256];
    char               message[256];

    int portNum = atoi (argv[3]);     // parse input parameter for port information
    char hostname[16];
    strcpy(hostname, argv[2]);

    sockfd = createSocket(hostname, portNum, &servaddr);     // create a streaming socket
    if (sockfd < 0) {
        exit (1);
    }

    printf ("Enter a message: ");
    fflush(stdout);
    fgets (message, 256, stdin);
    // replace new line with null character
    message[strlen(message)-1] = '\0';

    // send request to server
    //if (sendRequest (sockfd, "<echo>Hello, World!</echo>", &servaddr) < 0) {
    if (sendRequest (sockfd, message, &servaddr) < 0) {
        closeSocket (sockfd);
        exit (1);
    }

    if (receiveResponse(sockfd, response, 256) < 0) {
        closeSocket (sockfd);
        exit (1);
    }
    closeSocket (sockfd);

    // display response from server
    printResponse(response);

    /*-------------------------------------------------------------------------------------*/
    InitBBFile(argv[1]);

//    pthread_create(&networkThread, NULL, func, 0);
    while(PrintMenu()); //main/user thread for menu


    //token - command, selfIP, selfPort, nextIP, nextPort TODO

    //establish ring TODO

//    pthread_t tid;
//    pthread_create(&tid, NULL, );

    return 0;
}

//Constants
const int BUFFERSIZE = 256;

/*
 * Creates a datagram socket and connects to a server.
 *
 * serverName - the ip address or hostname of the server given as a string
 * serverPort - the port number of the server
 * dest       - the server's address information; the structure will be initialized with information
 *              on the server (like port, address, and family) in this function call
 *
 * return value - the socket identifier or a negative number indicating the error if a connection could not be established
 */
int createSocket(char * serverName, int port, struct sockaddr_in * dest)
{
    /*~~~~~~~~~~~~~~~~~~~~~Local vars~~~~~~~~~~~~~~~~~~~~~*/
    int socketFD;
    struct hostent *hostptr;
    struct in_addr ipAddress;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if( (socketFD = socket(AF_INET, SOCK_DGRAM, 0) ) < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    if( (hostptr = gethostbyname(serverName) ) == NULL)
    {
        perror("gethostbyname() failed, exit\n");
        return -1;
    }

    int i=0;
    printf("\nIP: " );
    while(hostptr->h_addr_list[i] != 0)
    {
        ipAddress.s_addr = *(u_long*)hostptr->h_addr_list[i++];
        printf("%s\n", inet_ntoa(ipAddress));
    }

    memset((void*)dest, 0, sizeof(struct sockaddr_in));    /* zero the struct */
    dest->sin_family = AF_INET;
    memcpy( (void *)&dest->sin_addr, (void *)hostptr->h_addr, hostptr->h_length);
    dest->sin_port = htons( (u_short)port );        /* set destination port number */
    printf("port: %d\n", htons(dest->sin_port));
    return socketFD;
}
/*
 * Sends a request for service to the server. This is an asynchronous call to the server,
 * so do not wait for a reply in this function.
 *
 * sockFD  - the socket identifier
 * request - the request to be sent encoded as a string
 * dest    - the server's address information
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int sendRequest(int socketFD, char * request, struct sockaddr_in * dest)
{
    socklen_t destSize = sizeof(struct sockaddr_in);
    int size = sendto(socketFD, request, strlen(request), 0, (struct sockaddr *) dest, destSize);
    return size;
}

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sockfd    - the socket identifier
 * response  - the server's response as an XML formatted string to be filled in by this function into the specified string array
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int receiveResponse(int socketFD, char * response, int size)
{
    int length = 0;
    socklen_t bufSize = (socklen_t)size;
    length = recvfrom(socketFD, response, bufSize, 0, NULL, NULL);
    response[length] = '\0';
    return length;
}

/*
 * Prints the response to the screen in a formatted way.
 *
 * response - the server's response as an XML formatted string
 *
 */
void printResponse(char * response)
{
    printf("%s\n",response);
}

/*
 * Closes the specified socket
 *
 * sockFD - the ID of the socket to be closed
 *
 * return - 0, if no error; otherwise, a negative number indicating the error
 */
int closeSocket(int socketFD)
{
    close(socketFD);
    return 0;
}