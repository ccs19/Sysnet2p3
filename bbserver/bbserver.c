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

#define MIN_HOSTS 1
#define MAX_HOSTS 10

typedef int bool;
enum{false, true};

typedef struct {
    struct addr_in * ip;
    int port;
}PeerInfo;

typedef struct {
    struct addr_in * senderIp;
    int port;
    bool haveToken;
}SendingInfo;

typedef struct{
    struct addr_in * machineExitIP;
    int machineExitPort;
    bool machineExit;
}TokenInfo;

//Globals
int numberOfHosts = -1;
int i;
int ServerSocket = 0;
struct hostent *HostByName = NULL;
struct sockaddr_in ServerAddress;
const int SERVER_PORT = 50000;
const int HostNameMaxSize = 256;
const int BUFFERSIZE = 256;
const char* ServerShutdownMessage = "Server shutting down...";
SendingInfo sendingInfo;

//function prototypes
void printNumberOfHosts(int numberOfArgs, const char *inputString);
void OpenSocket(int port);
void InitAddressStruct(int port);
void BindSocket();
void DisplayInfo();
void AcceptConnections();
void ExitOnError(char* errorMessage);
void HandleClientRequests(struct sockaddr_in* clientAddress);
void ParseClientMessage(char* clientMessage,  struct sockaddr_in* clientAddress, int clientSocket);
int XMLParser(  const char* beginXml,
        const char* endXml,
        char* clientMessage,
        char* token,
        int tokenSize);

void tellHost();

int main(int argc, char** argv)
{
    printNumberOfHosts(argc, argv[1]); //get number of desired hosts
    PeerInfo array[numberOfHosts];

    OpenSocket(SERVER_PORT);
    AcceptConnections();

    for(i = 0; i < numberOfHosts; ++i) //wait for hosts
    {
        //for number receive messages and stick in array, in_addr, int port
    }

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
    sendingInfo.haveToken = false;
//    sendingInfo.port = ;
//    sendingInfo.senderIp = ;


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
void DisplayInfo()
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
    /*~~~~~~~~~~~~~~~~~~~~~Local vars~~~~~~~~~~~~~~~~~~~~~*/
    struct sockaddr_in clientAddress;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    DisplayInfo();
    printf("Waiting for connection... ");
    fflush(stdout);
    for(;;)
    {
        HandleClientRequests(&clientAddress);
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
    @param  ClientSocketPtr -- A pointer to the client socket typecasted to a void*
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
    fflush(stdout);
    length = recvfrom(
            ServerSocket,                     //Server socket
            stringBuffer,                     //Buffer for message
            bufSize,             //Size of buffer
            0,                                //Flags
            (struct sockaddr*)clientAddress,  //Source address
            &clientAddressLength              //Size of source address
    );
    stringBuffer[length] = '\0';
    printf("Received message: %s\n", stringBuffer);
    ParseClientMessage(stringBuffer, clientAddress, length);
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
    int i = 0;
    char string[BUFFERSIZE]; //String to send back to client
    char token[BUFFERSIZE];  //Token to use for echo reply
    string[0] = '\0';
    const int NUMLOADAVG = 3; //Number of load averages queries
    socklen_t clientAddressLength = sizeof(struct sockaddr_in);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /*~~~~~~~~~~~~~~~~~~~~~Load avg response~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if(strcmp(clientMessage, "<loadavg/>") == 0)
    {
        double loadavg[NUMLOADAVG];
        getloadavg(loadavg, NUMLOADAVG); //Get load average and put in double array

        //Begin string format:
        strcat(string, "<replyLoadAvg>");
        for(i = 0; i < NUMLOADAVG; ++i)
        {
            char tempAvg[BUFFERSIZE];
            sprintf(tempAvg, "%lf:", loadavg[i]);
            strcat(string, tempAvg);              //Append string to string we return to client
        }
        strcat(string, "</replyLoadAvg>\0"); //End of string
    }
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        /*~~~~~~~~~~~~~~~~~~~~~Echo response~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    else if ((XMLParser("<echo>", "</echo>", clientMessage, token, sizeof(token))) == 1)
    {
        //Set return echo string
        strcat(string, "<reply>");
        strcat(string, token);
        strcat(string, "</reply>\0");
    }
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        /*~~~~~~~~~~~~~~~~~~~~~Shut down response~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    else if(strcmp(clientMessage, "<shutdown/>") == 0)
    {
        strcat(string, ServerShutdownMessage);
    }
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    else //Else we have an invalid format
        strcat(string, "<error>unknown format</error>\0");
    printf("Sending back %s\n\n", string);
    if(     (sendto(
            ServerSocket,                       //Client socket
            string,                             //String buffer to send to client
            strlen(string),                     //Length of buffer
            0,                                  //flags
            (struct sockaddr*)clientAddress,    //Destination
            clientAddressLength                 //Length of clientAddress

    )) < 0) //If sendto fails
        printf("Failed to send\n");
    if(strcmp(string, ServerShutdownMessage) == 0) //If server shutdown message received
    {
        close(ServerSocket);
        exit(1);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: XMLParser
    Parses an XML value and returns a token.
    @param begin           --      The expected beginning of an XML expression
    @param end             --      The expected ending of an XML expression
    @param token           --      The token extracted from the expression
    @param clientMessage   --      Message to parse
    @param length          --      Size of token
    @return                --      1 on success, 0 on failure, -1 if token is too large to fit
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int XMLParser(  const char* beginXml,
        const char* endXml,
        char* clientMessage,
        char* token,
        int tokenSize)
{
    //~~~~~~~~~~~~~Local vars ~~~~~~~~~~~~~~~~~~~~~~~//
    char tempString[strlen(clientMessage)];
    char *delimiter = NULL;
    int returnVal = 0;
    int i = 0;
    int foundDelimiter = 0;
    int beginXmlLength = strlen(beginXml);
    int endXmlLength = strlen(endXml);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    token[0] = '\0';
    memcpy(tempString, clientMessage, beginXmlLength); //Copy first part of clientMessage into temp for comparison.
    tempString[beginXmlLength] = '\0';
    if(strcmp(tempString, beginXml) == 0 ) //If beginXml is found
    {
        memcpy(tempString, clientMessage, strlen(clientMessage)); //Copy entire clientMessage
        for(i = 1; i < strlen(clientMessage); ++i) //Check for valid delimiter here
        {
            if(tempString[i] == '<')
            {
                delimiter = tempString+i;//Potential valid delimiter found. Point delimiter ptr to location.
                foundDelimiter = 1;
                break;
            }
        }
        if(foundDelimiter)
        {
            delimiter[endXmlLength] = '\0';//Set end of delimiter to null
            if (strcmp(delimiter, endXml) != 0) //If invalid delimiter
                returnVal = 0;
            else {
                returnVal = 1;//Set valid return
                char *tempToken = clientMessage + (strlen(beginXml)); //Set temporary token to end of starting delimiter
                strtok(tempToken, "<");
                if (strlen(tempToken) > tokenSize) returnVal = -1;              //If token is too large, return -1
                else if (strcmp(tempToken, endXml) == 0) token[0] = '\0';       //Else if empty token found
                else strcat(token, tempToken);                                  //Else put extracted token in variable
            }
        }
    }
    return returnVal;
}