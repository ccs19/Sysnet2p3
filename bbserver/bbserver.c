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

void printNumberOfHosts(int numberOfArgs, const char *inputString);

int main(int argc, char** argv)
{
    printNumberOfHosts(argc, argv[1]); //get number of desired hosts

    return 0;
}

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

//dynamic array of clientVar structs
//for number recevie messages and stick in array
//tell neighbors
//struct receive - in_addr, int port
//struct send - toIP, to port, fromIP, fromPort, int hasToken