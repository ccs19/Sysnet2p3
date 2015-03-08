/*
 * common.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h> //For gethostbyname()
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFERSIZE 256

//sockaddr_in
/*
    sin_family -> e.g. AF_INET
    sin_port -> port number
    sin_addr -> IP Address
*/
typedef struct{
    struct sockaddr_in neighborInfo;
    int hasToken;
    int machineHasExited; //reset when machine before exitingMachine gets token again
    struct sockaddr_in exitingMachineInfo;
    struct sockaddr_in exitingMachineNeighborInfo;
}SendingInfo;

void PrintSockaddr_in(struct sockaddr_in *addr);

/*
 * Displays contents of a SendingInfo struct.
 *
 * sendingInfo - the sending info
 *
 */
void PrintSendingInfo(SendingInfo *sendingInfo);

#endif