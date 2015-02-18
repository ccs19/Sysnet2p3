#include "bbWriter.h"


#include <stdio.h>

BBFile m_boardFile;


int main(int argc, const char* argv[])
{
    if(argc < 2 || argc > 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
    }
    else
    {
        if(OpenFile(argv[1] == 0)) return 0; //Try to open file
        PrintMenu();

    }
    PrintMenu();
    return 0;
}


//Updates file pointer to bottom of stream
//Return 1 on success, 0 on failure
int UpdateFile()
{


    return 0;
}


//Writes to file
//Return 1 on success, 0 on failure
int WriteFile()
{

    return 0;
}

//Reads a line from file based on the sequence number
//Return 1 on success, 0 on failure
int ReadFile(int sequenceNumber)
{

    return 0;
}


//Prints sequence numbers available from 1-n
//Returns 1 on success, 0 on failure
int PrintSequenceNumbers()
{

    return 0;
}


//Opens file for R/W operations
//Returns 1 on success, 0 on failure
int OpenFile(char* fileName)
{

    m_boardFile.file = fopen(fileName, "rw");
    if(m_boardFile.file == NULL) return 0;
    return 1;
}

void PrintMenu()
{
    printf(  "\n"
             "=====================================================================\n"
             "|                      Bulletin Board Options                       |\n"
             "=====================================================================\n"
             "1. write  :  Appends a new message to the end of the message board\n"
             "2. read # :  Read a particular message from the message board using\n"
             "             a message sequence number. # is the sequence number of\n"
             "             the message on the board.\n"
             "3. list   :  Displays the range of valid sequence numbers of messages\n"
             "             posted to the board\n"
             "4. exit   :  Closes the message board\n\n"
             "   Option : ");
}
