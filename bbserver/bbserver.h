#ifndef _BB_SERVER_H
#define _BB_SERVER_H

typedef int bool;
enum{false, true};

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

//function prototypes
void printNumberOfHosts(int numberOfArgs, const char *inputString);
void OpenSocket(int port);
void InitAddressStruct(int port);
void BindSocket();
void DisplayServerInfo();
void AcceptConnections();
void ExitOnError(char* errorMessage);
void HandleClientRequests(struct sockaddr_in* clientAddress);
void ParseClientMessage(char* clientMessage,  struct sockaddr_in* clientAddress, int clientSocket);
void DisplayPeerInfo();

void tellHost();

#endif