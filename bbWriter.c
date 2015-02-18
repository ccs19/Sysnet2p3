#include "bbWriter.h"


#include <stdio.h>

BBFile m_boardFile;

//User options:
#define WRITE '1'
#define READ '2'
#define LIST '3'
#define EXIT '4'



int main(int argc, const char* argv[])
{
    if(argc < 2 || argc > 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
    }
    else
    {
        if(OpenFile(argv[1] == 0)) return 0; //Try to open file
        InitBBFile();                        //init struct
        while(PrintMenu());
    }
    return 0;
}


//Updates file pointer to bottom of stream
//Return 1 on success, 0 on failure
int UpdateFile()
{
    m_boardFile.lastError = UpdateFailed;
    return 0;
}


//Writes to file
//Return 1 on success, 0 on failure
int WriteFile()
{
    m_boardFile.lastError = WriteFailed;
    return 0;
}

//Reads a line from file based on the sequence number
//Return 1 on success, 0 on failure
int ReadFile()
{
    m_boardFile.lastError = ReadFailed;
    return 0;
}


//Prints sequence numbers available from 1-n
//Returns 1 on success, 0 on failure
int PrintSequenceNumbers()
{
    m_boardFile.lastError = PrintSequenceFailed;
    return 0;
}


//Opens file for R/W operations
//Returns 1 on success, 0 on failure
int OpenFile(char* fileName)
{
    m_boardFile.file = fopen(fileName, "rw");
    if(m_boardFile.file == NULL) return 0;
    return 0;
}

int PrintMenu()
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
    int optionResult = GetOption();
    if(0 == optionResult)           //If something failed
    {
        PrintErrorMessage();        //Print error
    }
    else if(EXIT == optionResult)   //If exit
    {
        return 0;
    }
    else
    {
        return 1;   //Otherwise, return and run again
    }
}

//Returns 0 on error, EXIT
int GetOption() {
    char userOption;
    userOption = getchar();
    switch (userOption) {
        case WRITE:
            return WriteFile();
            break;
        case READ:
            return ReadFile();
            break;
        case LIST:
            return PrintSequenceNumbers();
            break;
        case EXIT:
            return EXIT;
            break;
        default:
            break;
    }
}

void InitBBFile()
{
    m_boardFile.lastError = -1;
    m_boardFile.file = NULL;
    m_boardFile.count = 0;
}

void PrintErrorMessage()
{

    switch(m_boardFile.lastError)
    {
        case UpdateFailed:
            break;
        case WriteFailed:
            break;
        case ReadFailed:
            break;
        case PrintSequenceFailed:
            break;
        default:
            break;
    }

}