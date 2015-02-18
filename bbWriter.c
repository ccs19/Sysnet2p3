/*
 * bbWriter.c
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 *
 */

#include "bbWriter.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BBFile m_boardFile;

//User options:
#define WRITE '1'
#define READ '2'
#define LIST '3'
#define EXIT '4'
#define INVALID '5'



//Constants
 const int MAX_MESSAGE_SIZE = 256;
 const char* endXml = "</message>";



int main(int argc, const char* argv[])
{
    if(argc < 2 || argc > 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
    }
    else
    {
        InitBBFile();
        if(0 == OpenFile(argv[1]))
        {

            printf("Failed to open file %s\n", argv[1]);
            return 0;
        } //Try to open file
        //init struct
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
    //=========================================================================================//
    char messageToWrite[MAX_MESSAGE_SIZE];          //Message to be written to BB
    char tempBuffer[MAX_MESSAGE_SIZE];              //Buffer to hold message temporarily to avoid overflow
    int messageSize = 0;                            //Size of message
    memset(messageToWrite, 0, MAX_MESSAGE_SIZE);
    memset(tempBuffer,0,MAX_MESSAGE_SIZE);
    //=========================================================================================//


    sprintf(messageToWrite, "<message n = %d>", m_boardFile.count++);
    messageSize += strlen(messageToWrite) + strlen(endXml);
    fgets(tempBuffer, MAX_MESSAGE_SIZE, stdin);             //Get user message here
    messageSize += strlen(tempBuffer);

    if(messageSize > MAX_MESSAGE_SIZE-1)                      //If user message + XML tags cause overflow
    {
        m_boardFile.lastError = WriteMessageBufferOverflow;
        return 0;
    }
    else
    {

        int messagePaddingLength = MAX_MESSAGE_SIZE - messageSize;  //Pad the end of message with spaces
        char messagePad[MAX_MESSAGE_SIZE];
        memset(messagePad, 0, MAX_MESSAGE_SIZE);
        sprintf(messagePad, "%*s\n", messagePaddingLength-1, ""); //length minus 1 to account for \n

        strcat(messageToWrite, messagePad);                     //Concat message to write with padding
        strcat(messageToWrite, tempBuffer);                     //Message to write
        strcat(messageToWrite, endXml);                         //And closing XML tag
        fseek(m_boardFile.file, 0, SEEK_END);

        int result = fprintf(m_boardFile.file, "%s\n", messageToWrite);
        if( result < 0 )
        {
            m_boardFile.lastError = WriteFailed;
            return 0;
        }
        else
        {

            return 1;
        }
    }
}



//Reads a line from file based on the sequence number
//Return 1 on success, 0 on failure
int ReadFileBySequenceNumber()
{
    int sequenceNumber;
    printf(" Sequence Number: ");
    fflush(stdout);
    int result = scanf("%d", &sequenceNumber); //TODO need to eat \n character
    if( result > 0                                    //Check that user input something
            && sequenceNumber <= m_boardFile.count+1  //and sequence number exists//TODO update this logic when we decide how to count sequenceNumbers
            && sequenceNumber > 0)                    //And sequence number greater than 0
    {
        if(fseek(m_boardFile.file, ( (sequenceNumber-1) /*Sequences start at line 0*/ * MAX_MESSAGE_SIZE), SEEK_SET) == 0)
        {
            char messageToPrint[MAX_MESSAGE_SIZE],
                 beginXml[MAX_MESSAGE_SIZE],
                 messageToParse[MAX_MESSAGE_SIZE];
            fread(messageToParse, sizeof(char), MAX_MESSAGE_SIZE, m_boardFile.file); //Get string of data to parse
            sprintf(beginXml, "<message n = %d>",  sequenceNumber);
            int xmlResult = XMLParser(beginXml, endXml, messageToParse, messageToPrint, MAX_MESSAGE_SIZE);
            if(xmlResult == 0)           //If invalid message in file
            {
                m_boardFile.lastError = InvalidXMLSyntax;
                return 0;
            }
            else if(xmlResult == -1)    //If message too large
            {
                m_boardFile.lastError = ReadMessageBufferOverflow;
                return 0;
            }
            printf("Message:\n%s", messageToPrint);
            return 1;
        }
        else //If seek fails...
        {
            m_boardFile.lastError = SeekFailed;
            return 0;
        }
    }
    else //Invalid user input or sequence number doesn't exist
    {
        m_boardFile.lastError = ReadFailed;
        return 0;
    }
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
int OpenFile(const char* fileName)
{
    m_boardFile.file = fopen(fileName, "a+");
    if(NULL == m_boardFile.file) return 0;
    return 1;
}




//Prints the BB Menu
//Returns 0 when user requests exit
//otherwise returns 1
int PrintMenu()
{
    m_boardFile.lastError = -1; //Reset error
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
    fflush(stdout);
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
    return 0;
}




//Returns 0 on error
//Returns EXIT if option 4 is selected
int GetOption() {
    int userOption, charCount = 1;

    userOption = getchar();

    if(charCount + EatInputUntilNewline() > 2)
    {
        userOption = INVALID;
    }
        switch (userOption)
        {                   //Breaks not needed. TODO: remove later
            case WRITE:
                return WriteFile();
                break;
            case READ:
                return ReadFileBySequenceNumber();
                break;
            case LIST:
                return PrintSequenceNumbers();
                break;
            case EXIT:
                return EXIT;
                break;
            case INVALID:
                m_boardFile.lastError = InvalidBoardOption;
                return 0;
                break;
                break;
            default:
                break;
        }
    return 0;
}






void InitBBFile()
{
    m_boardFile.lastError = -1;
    m_boardFile.file = NULL;
    m_boardFile.count = 1;
}






void PrintErrorMessage()
{
    printf("\nERROR:  ");
    switch(m_boardFile.lastError)
    {
        case UpdateFailed:
            printf("Update failed in UpdateFile()\n");
            break;
        case WriteFailed:
            printf("Write failed in WriteFile()\n");
            break;
        case ReadFailed:
            printf("Read failed in ReadFileBySequenceNumber()\n");
            break;
        case PrintSequenceFailed:
            printf("Print sequence failed in PrintSequenceNumbers()\n");
            break;
        case InvalidBoardOption:
            printf("Invalid Bulletin Board Option\n");
            break;
        case SeekFailed:
            printf("File seek operation failed in ReadFileBySequenceNumber()\n");
            break;
        case InvalidXMLSyntax:
            printf("Invalid syntax for selected sequence number\n");
            break;
        case ReadMessageBufferOverflow:
            printf("Read message buffer overflow. Message read exceeds max size of %d\n", MAX_MESSAGE_SIZE);
            break;
        case WriteMessageBufferOverflow:
            printf("Write message buffer overflow. Message write request exceeds max size of %d\n", MAX_MESSAGE_SIZE);
            break;
        default:
            break;
    }


}





/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: XMLParser

    Parses an XML value and returns a token.

    @param begin           --      The expected beginning of an XML expression
    @param end             --      The expected ending of an XML expression
    @param clientMessage   --      Message to parse
    @param token           --      The token extracted from the expression
    @param length          --      Size of token
    @return                --      1 on success, 0 on failure, -1 if token is too large to fit
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int XMLParser(  const char* beginXml,
        const char* endXml,
        char* clientMessage,
        char* token,
        int tokenSize)
{
    //~~~~~~~~~~~~~Local vars ~~~~~~~~~~~~~~~~~~~~~~~//
    int clientMessageLength = strlen(clientMessage);
    int beginXmlLength = strlen(beginXml);
    int endXmlLength = strlen(endXml);
    char tempString[clientMessageLength];
    char *delimiter = NULL;
    int returnVal = 0;
    int i = 0;
    int foundDelimiter = 0;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    token[0] = '\0';
    memcpy(tempString, clientMessage, beginXmlLength); //Copy first part of clientMessage into temp for comparison.
    tempString[beginXmlLength] = '\0';
    if(strcmp(tempString, beginXml) == 0 ) //If beginXml is found
    {
        memcpy(tempString, clientMessage, clientMessageLength); //Copy entire clientMessage
        for(i = 1; i < clientMessageLength; i++) //Check for valid delimiter here
        {
            if(tempString[i] == '<')
            {
                delimiter = tempString+i;//Potential valid delimiter found. Point delimiter ptr to location.
                foundDelimiter = 1;
                break;
            }
        }
        if(foundDelimiter)
        {
            delimiter[endXmlLength] = '\0';//Set end of delimiter to null
            if (strcmp(delimiter, endXml) != 0) //If invalid delimiter
                returnVal = 0;
            else {
                returnVal = 1;//Set valid return
                char *tempToken = clientMessage + beginXmlLength; //Set temporary token to end of starting delimiter
                strtok(tempToken, "<");
                if (strlen(tempToken) > tokenSize) returnVal = -1;              //If token is too large, return -1
                else if (strcmp(tempToken, endXml) == 0) token[0] = '\0';       //Else if empty token found
                else strcat(token, tempToken);                                  //Else put extracted token in variable
            }
        }
    }
    return returnVal;
}

//NOM NOMs all user input until a newline is found.
//Returns the number of characters ate
int EatInputUntilNewline()
{
    int charCount = 0;
    while(getchar() != '\n')//Consume newline
    {
        charCount++;
    }
    return charCount;
}