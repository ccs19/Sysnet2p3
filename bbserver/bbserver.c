/*
 * bbserver.c
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 *
 */

#include <stdio.h>
#include <stdlib.h>

int numberOfHosts = -1;

#define MIN_HOSTS 1
#define MAX_HOSTS 10

typedef struct
{
    int port;
} HostInfo;

typedef struct
{
    int hasToken;
    HostInfo host;
} MessageForHost;

void printNumberOfHosts(int numberOfArgs, const char *inputString);

int main(int argc, char** argv)
{
    printNumberOfHosts(argc, argv[1]); //get number of desired hosts

    //dynamic array of clientVar structs
    //for number recevie messages and stick in array
    //tell neighbors
    //struct receive - in_addr, int port
    //struct send - toIP, to port, fromIP, fromPort, int hasToken

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
    }
    else if(numberOfHosts > MAX_HOSTS)
    {
        printf("%d hosts invalid. At least %d and at most %d hosts required.\n", numberOfHosts, MIN_HOSTS, MAX_HOSTS);
    }
    else
    {
        printf("Number of hosts: %d\n", numberOfHosts);
    }
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
