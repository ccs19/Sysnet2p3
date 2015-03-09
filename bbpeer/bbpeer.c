/*
 * bbpeer.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#include <errno.h> // for threading
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include "bbpeer.h"
#include "bbwriter.h"
#include "../common.h"

pthread_t networkThread; //thread for token passing
pthread_t mainThread;    //operates the menu
int loopMenu = 1;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: main

    Accepts a file in argv[1]. The main/user thread operates the menu. The network thread handles
    IO and token passing and receiving.

    @param argc         --  number of args
    @param argv         --  arg vector
 */
void AcquireToken(SendingInfo *info, int mySocket, int neighborSocket, socklen_t *sockAddrLength, char stringBuffer[]);
void ServerSetup(int numArgs, const char *programName, const char *nameOfServer, const char *portString, SendingInfo *info);
void ChooseTokenHolder(SendingInfo *info, int mySocket, int neighborSocket, socklen_t *sockAddrLength, char stringBuffer[]);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, const char* argv[])
{
    InitBBFile(argv[1]);

    SendingInfo* info = malloc(sizeof(SendingInfo));
    ServerSetup(argc, argv[0], argv[2], argv[3], info);

    pthread_create(&networkThread, NULL, (void *)InitNetworkThread, (void*)(info));

    while(loopMenu);                        //prevent joining of threads until it's time
    pthread_join(mainThread, NULL);         //join threads
    pthread_join(networkThread, NULL);
    return 0;
}

/*
 * Chooses first token holder.
 */
void ChooseTokenHolder(SendingInfo *info, int mySocket, int neighborSocket, socklen_t *sockAddrLength, char stringBuffer[])
{
    srand(time(NULL) + info->exitingMachineInfo.sin_port); //Init random seed
    int myRandomNumber = rand();
    int highestRandomNumber = 0;
    int loopNum = 0;
    const int TOKEN_SET = -1;

    printf("My random number %d\n", myRandomNumber + info->exitingMachineInfo.sin_port);

    while(1) {
        int neighborNumber;
        if(0 == loopNum) highestRandomNumber = myRandomNumber; //If first time through loop

        if(0 == loopNum ||                              //If first time through the loop
                highestRandomNumber > myRandomNumber);  //Or forward highest number to neighbor
        {
            printf("Sending %d\n", highestRandomNumber);
            sendto(
                    neighborSocket,                              //Client socket
                    (void *)&highestRandomNumber,                //Current highest random number
                    sizeof(int),                                 //Length of buffer
                    0,                                           //flags
                    (struct sockaddr *) &info->neighborInfo,     //Destination
                    (*sockAddrLength)                            //Length of clientAddress
            );
        }
        recvfrom(
                mySocket,                     //Server socket
                (void*)&neighborNumber,                     //Buffer for message
                sizeof(int),             //Size of buffer
                0,                                //Flags
                (struct sockaddr *) &info->exitingMachineInfo,  //Source address
                sockAddrLength             //Size of source address
        );
        if(neighborNumber > myRandomNumber)
        {
            highestRandomNumber = neighborNumber;
        }
        else if(TOKEN_SET == neighborNumber) //-1 is signal to exit loop
        {
            info->hasToken = 0;
            sendto(
                    neighborSocket,                              //Client socket
                    (void *)&TOKEN_SET,                          //Token is initialized
                    sizeof(int),                                 //Length of buffer
                    0,                                           //flags
                    (struct sockaddr *) &info->neighborInfo,     //Destination
                    (*sockAddrLength)                            //Length of clientAddress
            );
            printf("I don't have the token\n");
            break;                    //Break out of while loop
        }
        else if(neighborNumber == myRandomNumber) //My number is highest, so I get the token first
        {
            info->hasToken = 1;
            sendto(
                    neighborSocket,                              //Client socket
                    (void *)&TOKEN_SET,                          //Token is initialized
                    sizeof(int),                                 //Length of buffer
                    0,                                           //flags
                    (struct sockaddr *) &info->neighborInfo,     //Destination
                    (*sockAddrLength)                            //Length of clientAddress
            );

            recvfrom(
                    mySocket,                     //Server socket
                    (void *)&neighborNumber,                     //Buffer for message
                    sizeof(int),             //Size of buffer
                    0,                                //Flags
                    (struct sockaddr *) &info->exitingMachineInfo,  //Source address
                    sockAddrLength             //Size of source address
            ); //Wait for all clients to receive signal to exit

            printf("I have the token\n");
            break; //Token ring initialized
        }


        printf("Received %d, highest number is %d\n", neighborNumber, highestRandomNumber );

        loopNum++;

    }
}

/*
 * Performs communication with server to find out who the neighbor is.
 */
void ServerSetup(int numArgs, const char *programName, const char *nameOfServer, const char *portString, SendingInfo *info)
{
    if(numArgs != 4)
    {
        printf("Usage: %s <filename> <serverIP> <serverPort>\n", programName);
        exit(1);
    }

    int serverSocketFD, serverPortNum;
    struct sockaddr_in serverAddress;
    char serverName[256];

    memset(info, 0, sizeof(SendingInfo));

    serverPortNum = atoi (portString);     // parse input parameter for port information
    strcpy(serverName, nameOfServer);       // get server hostname

    serverSocketFD = createSocket(serverName, serverPortNum, &serverAddress);     // create a streaming socket

    printf ("Sending info to server...\n");

    sendRequest(serverSocketFD, "", &serverAddress);
    receiveServerResponseAndClose(serverSocketFD, info, sizeof(SendingInfo)); //closes socket when received
}

/*
 * Runs the menu endlessly until the peer exits.
 */
void MenuRunner()
{
    int loop = 1;
    while (loop)
    {
        loop = PrintMenu();
    }
    loopMenu = 0;
}

/*
 * Initializes network thread.
 */
void InitNetworkThread(void* pInfo)
{
    SendingInfo* info = (SendingInfo*)pInfo;
    int mySocket;
    int neighborSocket;
    socklen_t sockAddrLength = sizeof(struct sockaddr);
    char stringBuffer[BUFFERSIZE];
    stringBuffer[0] = '\0';
//    strcat(stringBuffer, "Hello!"); //TODO delete

    //Init sockets
    OpenSocket(0, &mySocket, &info->exitingMachineInfo);
    neighborSocket = createSocket(inet_ntoa(info->neighborInfo.sin_addr), ntohs(info->neighborInfo.sin_port), &info->neighborInfo);
    sleep(1); //Give time for all peers to open socket

//    AcquireToken(info, mySocket, neighborSocket, &sockAddrLength, stringBuffer);  //TODO where to call this?
    ChooseTokenHolder(info, mySocket, neighborSocket, &sockAddrLength, stringBuffer);

    printf("Message: %s\n", stringBuffer);
    PrintSendingInfo(info);

    fflush(stdout);
    pthread_create(&mainThread, NULL, (void*)&MenuRunner, (void*)&(info)); //shouldn't come up until token passing begins
}

/*
 * Acquire token, do stuff, send on.
 */
void AcquireToken(SendingInfo *info, int mySocket, int neighborSocket, socklen_t *sockAddrLength, char stringBuffer[])
{
    int length;
    puts("Acquire: about to receive");

    length = recvfrom(
            mySocket,                     //Server socket
            stringBuffer,                     //Buffer for message
            256,             //Size of buffer
            0,                                //Flags
            (struct sockaddr*)&info->exitingMachineInfo,  //Source address
            sockAddrLength             //Size of source address
    );

    puts("Acquire: about to send");
    //do stuff now that I have the token TODO
    //think about having a queue of commands that gets run when this happens

    sendto(
            neighborSocket,                //Client socket
            (void*)stringBuffer,           //String buffer to send to client
            256,                       //Length of buffer
            0,                                  //flags
            (struct sockaddr*)&info->neighborInfo,    //Destination
            (*sockAddrLength)                 //Length of clientAddress
    );

    puts("Acquire: send complete");
}

/*
 * Open socket and bind.
 */
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

/*
 * Binds a socket.
 */
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
void receiveServerResponseAndClose(int socketFD, SendingInfo *response, int size)
{
    socklen_t bufSize = sizeof(SendingInfo);
    int length = (int)recvfrom(socketFD, response, bufSize, 0, NULL, NULL);
    if(length < 0)
    {
        closeSocket(socketFD);
        exit(1);
    }

    PrintSendingInfo(response);
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