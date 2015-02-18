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
 const int MAX_MESSAGE_SIZE = 1024;
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
    m_boardFile.lastError = WriteFailed;
    return 0;
}



//Reads a line from file based on the sequence number
//Return 1 on success, 0 on failure
int ReadFileBySequenceNumber()
{
    int sequenceNumber;
    printf(" Sequence Number: ");
    fflush(stdout);
    int result = scanf("%d", &sequenceNumber);
    if( result > 0 && //Check for user input
            sequenceNumber <= m_boardFile.count+1 || sequenceNumber > 0) //and Check if sequence number exists TODO update this logic when we decide how to count sequenceNumbers
    {
        if(fseek(m_boardFile.file, ( (sequenceNumber-1) * MAX_MESSAGE_SIZE), SEEK_SET) == 0)// (file, offset(messagenumber * fixed message size), origin(SEEK_SET = beginning of file))
        {
            char messageToPrint[MAX_MESSAGE_SIZE],
                 beginXml[MAX_MESSAGE_SIZE],
                 messageToParse[MAX_MESSAGE_SIZE];
            fread(messageToParse, sizeof(char), MAX_MESSAGE_SIZE, m_boardFile.file); //Get string of data to parse
            sprintf(beginXml, "<message n=%d>",  sequenceNumber);
            XMLParser(beginXml, endXml, messageToParse, messageToPrint, MAX_MESSAGE_SIZE);
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
    m_boardFile.file = fopen(fileName, "rw");
    if(NULL == m_boardFile.file) return 0;
    return 1;
}




//Prints the BB Menu
//Returns 0 when user requests exit
//otherwise returns 1
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

    while(getchar() != '\n')//Consume newline
    {
        if(++charCount >= 2) //check for input of more than two characters
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
        default:
            break;
    }
    return 0;
}






void InitBBFile()
{
    m_boardFile.lastError = -1;
    m_boardFile.file = NULL;
    m_boardFile.count = 5;
}






void PrintErrorMessage()
{
    printf("\nERROR:  ");
    switch(m_boardFile.lastError)
    {
        case UpdateFailed:
            printf("Update failed\n");
            break;
        case WriteFailed:
            printf("Write failed\n");
            break;
        case ReadFailed:
            printf("Read failed\n");
            break;
        case PrintSequenceFailed:
            printf("Print sequence failed\n");
            break;
        case InvalidBoardOption:
            printf("Invalid Bulletin Board Option\n");
            break;
        case SeekFailed:
            printf("File seek operation failed\n");
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
    @param token           --      The token extracted from the expression
    @param clientMessage   --      Message to parse
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
    char tempString[strlen(clientMessage)];
    char *delimiter = NULL;
    int returnVal = 0;
    int i = 0;
    int foundDelimiter = 0;
    int beginXmlLength = strlen(beginXml);
    int endXmlLength = strlen(endXml);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    token[0] = '\0';
    memcpy(tempString, clientMessage, beginXmlLength); //Copy first part of clientMessage into temp for comparison.
    tempString[beginXmlLength] = '\0';
    if(strcmp(tempString, beginXml) == 0 ) //If beginXml is found
    {
        memcpy(tempString, clientMessage, strlen(clientMessage)); //Copy entire clientMessage
        for(i = 1; i < strlen(clientMessage); i++) //Check for valid delimiter here
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
                char *tempToken = clientMessage + (strlen(beginXml)); //Set temporary token to end of starting delimiter
                strtok(tempToken, "<");
                if (strlen(tempToken) > tokenSize) returnVal = -1;              //If token is too large, return -1
                else if (strcmp(tempToken, endXml) == 0) token[0] = '\0';       //Else if empty token found
                else strcat(token, tempToken);                                  //Else put extracted token in variable
            }
        }
    }
    return returnVal;
}