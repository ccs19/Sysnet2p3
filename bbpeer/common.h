/*
 * common.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#ifndef _COMMON_H
#define _COMMON_H


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

#endif