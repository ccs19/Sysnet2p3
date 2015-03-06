/*
 * bbpeer.h
 * Systems and Networks II
 * Project 2
 *
 * This file describes the functions to be implemented by the UDPclient.
 * You may also implement any helper functions you deem necessary to complete the program.
 */

#include <netinet/in.h>

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
int createSocket(char*, int, struct sockaddr_in *);

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
int sendRequest(int, char*, struct sockaddr_in*);

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sockfd    - the socket identifier
 * response  - the server's response as an XML formatted string to be filled in by this function into the specified string array
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int receiveResponse(int, char*, int);

/*
 * Prints the response to the screen in a formatted way.
 *
 * response - the server's response as an XML formatted string
 *
 */
void printResponse(char*);

/*
 * Closes the specified socket
 *
 * sockFD - the ID of the socket to be closed
 *
 * return - 0, if no error; otherwise, a negative number indicating the error
 */
int closeSocket(int);