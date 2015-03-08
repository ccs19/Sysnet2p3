#ifndef _BB_SERVER_H
#define _BB_SERVER_H





//function prototypes
void printNumberOfHosts(int numberOfArgs, const char *inputString);
void OpenSocket(int port);
void InitAddressStruct(int port);
void BindSocket();
void DisplayServerInfo();
void AcceptConnections();
void ExitOnError(char* errorMessage);
void HandleClientRequests(struct sockaddr_in* clientAddress);
void DisplayPeerInfo();


#endif