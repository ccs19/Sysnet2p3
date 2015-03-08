/*
 * common.c
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

#include "common.h"

void PrintSockaddr_in(struct sockaddr_in *addr)
{
    printf("IP: %s Port: %d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

/*
 * Displays contents of a SendingInfo struct.
 *
 * sendingInfo - the sending info
 *
 */
void PrintSendingInfo(SendingInfo *sendingInfo)
{
    printf("Neighbor info: ");
    PrintSockaddr_in(&sendingInfo->neighborInfo);
    printf("Has token: %d\n", sendingInfo->hasToken);
    printf("Machine has exited: %d\n", sendingInfo->machineHasExited);
    printf("Exiting Machine Info ");
    PrintSockaddr_in(&sendingInfo->exitingMachineInfo);
    printf("Exiting Machine Display Info: ");
    PrintSockaddr_in(&sendingInfo->exitingMachineNeighborInfo);
}