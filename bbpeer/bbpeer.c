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
#include "bbwriter.h"
#include "common.h"

ServerInfo serverInfo;

//Constants
// const int MAX_MESSAGE_SIZE = 256;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: main

    Accepts a file in argv[1]. The main/user thread operates the menu. The network thread handles
    IO and token passing and receiving.

    @param argc         --  number of args
    @param argv         --  arg vector
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, const char* argv[])
{
    if(argc != 4)
    {
        printf("Usage: %s <filename> <serverIP> <serverPort>\n", argv[0]);
        return -1;
    }

    pthread_t networkThread; //thread for token and IO
    pthread_t mainThread;    //operates the menu
    InitBBFile(argv[1]);

    int serverSocketFD;
    int neighborSocketFD;
    struct sockaddr_in serverAddress;

    SendingInfo* info = malloc(sizeof(SendingInfo));
    memset(info, 0, sizeof(SendingInfo));
    char message[256];

    int portNum = atoi (argv[3]);     // parse input parameter for port information
    char serverName[256];
    strcpy(serverName, argv[2]);       // get server hostname

    serverSocketFD = createSocket(serverName, portNum, &serverAddress);     // create a streaming socket

    printf ("Sending info to server...");
    fflush(stdout);
    fgets (message, 256, stdin);
    message[strlen(message) - 1] = '\0';     // replace new line with null character

    sendRequest(serverSocketFD, message, &serverAddress);
    receiveServerResponse(serverSocketFD, info, sizeof(SendingInfo)); //closes socket when received
    
    sprintf(message, "Hey neighbor!"); //Do stuff with your neighbor
    
    pthread_create(&mainThread, NULL, (void*)&MenuRunner, (void*)&(info)); //shouldn't come up until token passing begins

    pthread_create(&networkThread, NULL, (void *)receiveMessage, (void*)&(info->exitingMachineInfo));
    neighborSocketFD = createSocket(inet_ntoa(info->neighborInfo.sin_addr), ntohs(info->neighborInfo.sin_port), &info->neighborInfo);
    sleep(5);
    sendto(
            neighborSocketFD,                //Client socket
            (void*)message,           //String buffer to send to client
            256,                       //Length of buffer
            0,                                  //flags
            (struct sockaddr*)&info->neighborInfo,    //Destination
            sizeof(struct sockaddr)                 //Length of clientAddress
        );
        
    //establish ring TODO
    //join threads TODO
    while(1);
    return 0;
}

void MenuRunner(void * pInfo)
{
    puts("I'm in menu runner!!!");
    SendingInfo* info = (SendingInfo *)pInfo;

    while (!info->machineHasExited)
    {
       info->machineHasExited = PrintMenu();
    }
}

void receiveMessage(void* myInfo)
{
    struct sockaddr_in* sockAddrnInfo = (struct sockaddr_in*)myInfo;
    int mySocket;
    int length;
    OpenSocket(0, &mySocket, sockAddrnInfo);
    char stringBuffer[256];
    socklen_t socketLength = sizeof(sockAddrnInfo);
    
    stringBuffer[0] = '\0';
        length = recvfrom(
            mySocket,                     //Server socket
            stringBuffer,                     //Buffer for message
            256,             //Size of buffer
            0,                                //Flags
            (struct sockaddr*)sockAddrnInfo,  //Source address
            &socketLength              //Size of source address
    );
    
    printf("My message is: %s\n", stringBuffer);
    fflush(stdout);
}

void OpenSocket(int port, int* mySocket, struct sockaddr_in* sockAddrnInfo)
{
    char hostname[256];

    if( ( *mySocket =  socket(PF_INET, SOCK_DGRAM, 0) ) <  0)  //If socket fails
        printf("Error creating socket");

    if(gethostname(hostname, sizeof(hostname)) < 0)               //If getting hostname fails
        printf("Error acquiring hostname. Exiting");

    //InitAddressStruct(port); TODO
    BindSocket(mySocket, sockAddrnInfo);
    
}

/*void InitAddressStruct(int port)
{
    memset((void*)&ServerAddress, '0', (size_t)sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    memcpy( (void *)&ServerAddress.sin_addr, (void *)HostByName->h_addr, HostByName->h_length);
    ServerAddress.sin_port = htons(port);
}*/

void BindSocket(int* mySocket, struct sockaddr_in* sockAddrnInfo)
{
    
    if( ( bind( *mySocket, (struct sockaddr *)sockAddrnInfo, sizeof(struct sockaddr_in)) )  < 0)
        printf("Failed to bind socket"); //If binding of socket fails
}

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
        exit(1);
    }
    if( (hostptr = gethostbyname(serverName) ) == NULL)
    {
        perror("gethostbyname() failed, exit\n");
        exit(1);
    }

    int i = 0;
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
    printf("port: %d\n", ntohs(dest->sin_port));
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
 */
void sendRequest(int socketFD, char * request, struct sockaddr_in * dest)
{
    socklen_t destSize = sizeof(struct sockaddr_in);
    int size = sendto(socketFD, request, strlen(request), 0, (struct sockaddr *) dest, destSize);
    if(size < 0)
    {
        closeSocket(socketFD);
        exit(1);
    }
}

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sockfd    - the socket identifier
 * response  - the server's response as an XML formatted string to be filled in by this function into the specified string array
 *
 */
void receiveServerResponse(int socketFD, SendingInfo* response, int size)
{
    socklen_t bufSize = sizeof(SendingInfo);
    int length = (int)recvfrom(socketFD, response, bufSize, 0, NULL, NULL);
    if(length < 0)
    {
        closeSocket(socketFD);
        exit(1);
    }

    printf("Neighbor IP: %s Port: %d\n", 
        inet_ntoa(response->neighborInfo.sin_addr), 
        ntohs(response->neighborInfo.sin_port));
        
    closeSocket(socketFD);
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
    int returnVal = close(socketFD);
    return returnVal;
}