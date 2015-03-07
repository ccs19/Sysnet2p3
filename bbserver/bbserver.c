/*
 * bbserver.c
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h> //For gethostbyname()
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> //for memset

#include "bbserver.h"

#define MIN_HOSTS 1
#define MAX_HOSTS 10

//Globals
int numberOfHosts = -1;
int i;
int ServerSocket = 0;
struct hostent *HostByName = NULL;
struct sockaddr_in ServerAddress;
struct sockaddr_in peerInfo[10];

const int SERVER_PORT = 50000;
const int HostNameMaxSize = 256;
const int BUFFERSIZE = 256;
const char* ServerShutdownMessage = "Server shutting down...";

int main(int argc, char** argv)
{
    OpenSocket(SERVER_PORT);
    printNumberOfHosts(argc, argv[1]);  //get number of desired hosts
    DisplayServerInfo();                //displays info about self
    AcceptConnections();                //captures all peer info
    DisplayPeerInfo();                  //displays info about all peers

    for(i = 0; i < numberOfHosts; ++i) //tell hosts
    {
        tellHost();     //send - toIP, to port
    }

    return 0;
}

/*
 * Checks that a command line argument is a valid integer number of hosts. If valid, it is printed. Otherwise,
 * the program exits.
 *
 * numberOfArgs    - number of command line args
 * inputString     - hopefully, a valid integer number of hosts
 *
 */
void printNumberOfHosts(int numberOfArgs, const char *inputString)
{
    if(numberOfArgs != 2)
    {
        printf("Usage: ./bbserver <number of hosts>\n");
        exit(1);
    }

    numberOfHosts = atoi(&inputString[0]);

    if(numberOfHosts < MIN_HOSTS)
    {
        printf("%d host invalid. At least %d and at most %d hosts required.\n", numberOfHosts, MIN_HOSTS, MAX_HOSTS);
        exit(1);
    }
    else if(numberOfHosts > MAX_HOSTS)
    {
        printf("%d hosts invalid. At least %d and at most %d hosts required.\n", numberOfHosts, MIN_HOSTS, MAX_HOSTS);
        exit(1);
    }
    else
    {
        printf("Number of hosts: %d\n", numberOfHosts);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   OpenSocket
    Opens the listening socket and resolves the host name.
    @param  port           -- The port number to bind the listen socket
    @return                -- void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void OpenSocket(int port)
{
    char hostname[HostNameMaxSize];

    if( ( ServerSocket =  socket(PF_INET, SOCK_DGRAM, 0) ) <  0)  //If socket fails
        ExitOnError("Error creating socket");

    if(gethostname(hostname, sizeof(hostname)) < 0)               //If getting hostname fails
        ExitOnError("Error acquiring hostname. Exiting");

    if( ( HostByName = gethostbyname(hostname) ) ==  NULL)        //If gethostbyname fails print error message, exit
    {
        herror("GetHostByName failed. Error: ");
        exit(1);
    }
    InitAddressStruct(port);
    BindSocket();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   InitAddressStruct
    Initializes the ServerAddress structure with the IP, port, and protocol type.
    @param  port           --  Port number to open
    @return                --  void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void InitAddressStruct(int port)
{
    memset((void*)&ServerAddress, '0', (size_t)sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    memcpy( (void *)&ServerAddress.sin_addr, (void *)HostByName->h_addr, HostByName->h_length);
    ServerAddress.sin_port = htons(port);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   BindSocket
    Binds the server socket
    @return           --    void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void BindSocket()
{
    if( ( bind( ServerSocket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) )  < 0)
        ExitOnError("Failed to bind socket"); //If binding of socket fails
}

/*
 * Tells a host...
 *
 */
void tellHost()
{
    
    //last: to = 1, token = 1
    //else: to = self + 1, token = 0
}

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sockfd    - the socket identifier
 * response  - the server's response as an XML formatted string to be filled in by this function into the specified string array
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int receiveHostMessage(int socketFD, char * response, int size)
{
    int length = 0;
    socklen_t bufSize = (socklen_t)size;
    length = recvfrom(socketFD, response, bufSize, 0, NULL, NULL);
    response[length] = '\0';
    return length;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   DisplayInfo
    Displays the connection info of the server.
    @return           --    void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DisplayServerInfo()
{
    int i = 0;
    struct in_addr ipAddress;
    struct sockaddr_in sockIn;
    int sockLen = sizeof(sockLen);
    getsockname(ServerSocket, (struct sockaddr*)&sockIn, (socklen_t*)&sockLen );
    printf("Host Name: %s\n",HostByName->h_name);
    printf("IP:        ");
    while(HostByName->h_addr_list[i] != 0)
    {
        ipAddress.s_addr = *(u_long*)HostByName->h_addr_list[i++];
        printf("%s\n", inet_ntoa(ipAddress));
    }
    printf("Port:      %d\n", htons(sockIn.sin_port));
    fflush(stdout);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   AcceptConnections
    Accepts the client connection and creates a new detached thread for the client.
    @return           --    void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AcceptConnections()
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    int i = 0;
    printf("Waiting for %d connections... \n", numberOfHosts - i);
    while(i < numberOfHosts)
    {
        /*~~~~~~~~~~~~~~~~~~~~~Local vars~~~~~~~~~~~~~~~~~~~~~*/
        HandleClientRequests(&peerInfo[i]);
        ++i;
        printf("Waiting for %d connections... \n", numberOfHosts - i);
    }
    //Fill in structs with their neighbor info here
    
    for(i = 0; i < numberOfHosts; i++)
    {
        SendingInfo* pInfo = malloc(sizeof(SendingInfo));
        pInfo->neighborInfo = peerInfo[(i+1) % numberOfHosts];
        pInfo->hasToken = 0;
        pInfo->machineHasExited = 0;
        memcpy(&(pInfo->exitingMachineInfo), (void*)&(peerInfo[i]), sizeof(struct sockaddr_in)); //Set this to the peer's own info
        memset(&(pInfo->exitingMachineNeighborInfo), 0, sizeof(struct sockaddr_in));
        sendto
        (
            ServerSocket,                       //Client socket
            (void*)pInfo,                       //String buffer to send to client
            sizeof(SendingInfo),                //Length of buffer
            0,                                  //flags
            (struct sockaddr*)&peerInfo[i],     //Destination
            sizeof(peerInfo[i])                 //Length of clientAddress
        );
        free(pInfo);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   ExitOnError
    Prints a message to stdout and exits
    @param  errorMessage           -- Error message to be printed
    @return                        -- void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ExitOnError(char* errorMessage)
{
    printf("%s\n", errorMessage);
    exit(1);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   HandleClientRequests
    Depending on the string received from the client, we either print that it failed or parse
    the message, then close the socket and free the thread.
    @param  ClientSocketPtr -- A pointer to the client socket typecasted to a void *
    @return           -- void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HandleClientRequests(struct sockaddr_in* clientAddress)
{
    fflush(stdout);

    /*~~~~~~~~~~~~~~~~~~~~~Local vars~~~~~~~~~~~~~~~~~~~~~*/
    char stringBuffer[BUFFERSIZE];
    bzero(stringBuffer, BUFFERSIZE);
    socklen_t clientAddressLength = sizeof(clientAddress);
    socklen_t bufSize = BUFFERSIZE;
    int length;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    length = recvfrom(
            ServerSocket,                     //Server socket
            stringBuffer,                     //Buffer for message
            bufSize,             //Size of buffer
            0,                                //Flags
            (struct sockaddr*)clientAddress,  //Source address
            &clientAddressLength              //Size of source address
    );
    
    if(length < 0)
    {
        fprintf(stderr, "rcvfrom() failed.\n");
    }

    char clientAddressString[16];
    char clientPortString[6];

    sprintf(clientAddressString, "%s", inet_ntoa(clientAddress->sin_addr));
    sprintf(clientPortString, "%hu", htons(clientAddress->sin_port));

    printf("Client IP: %s\n", clientAddressString);
    printf("Client port: %s\n", clientPortString);
    puts("");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   DisplayPeerInfo
    Displays info of all peers
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DisplayPeerInfo()
{
    int i;
    for(i = 0; i < numberOfHosts; i++)
    {
        printf("Peer #%d IP: %s Port: %d\n", i, inet_ntoa(peerInfo[i].sin_addr), ntohs(peerInfo[i].sin_port));
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION:   ParseClientMessage
    Handles the message for the client and sends a message back to the client
    @param  clientMessage        -- Pointer to message received by client
    @param  ClientSocket         -- The socket to the client
    @return                      -- void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ParseClientMessage(char* clientMessage,  struct sockaddr_in* clientAddress, int clientSocket)
{
    /*~~~~~~~~~~~~~~~~~~~~~Local vars~~~~~~~~~~~~~~~~~~~~~*/
    socklen_t clientAddressLength = sizeof(struct sockaddr_in);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    //Initialize struct to return to client here
    size_t sendingInfoLength = sizeof(SendingInfo);
    SendingInfo sendingInfo;
    // sendingInfo.someValue = 65781685; //neighbor
    sendingInfo.hasToken = -1;
    sendingInfo.machineHasExited = 0;
    
    if(     (sendto(
            ServerSocket,                //Client socket
            (void*)&sendingInfo,           //String buffer to send to client
            sendingInfoLength,                       //Length of buffer
            0,                                  //flags
            (struct sockaddr*)clientAddress,    //Destination
            clientAddressLength                 //Length of clientAddress

    )) < 0) //If sendto fails
        {
        printf("Failed to send\n");
        }
}