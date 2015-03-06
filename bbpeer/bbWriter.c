/*
 * bbWriter.c
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

#include "bbWriter.h"

//Globals
BBFile m_boardFile;

//User options:
#define ERROR 0
#define WRITE '1'
#define READ '2'
#define LIST '3'
#define EXIT '4'
#define INVALID '5'

//Constants
 const int MAX_MESSAGE_SIZE = 256;
 const char * endXml = "</message>\n";
 const int lengthendXML = 11;
 const int READ_STRING_LENGTH = 4;          //Length of read string
 const int READ_SPACE = 4;                  //Array index of where we expect a space to be
 const int READ_SEQUENCE_NUMBER = 5;        //Array index of where we expect the sequence number to be

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: UpdateFile

    Reads the file and updates the message nextMessageNumber variable.

    @return     -- Number of next message. On failure, lastError is set and 0 returned.
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Updates file pointer to bottom of stream
int UpdateFile()
{
    int count = 1;
    if(fseek(m_boardFile.file, 0, SEEK_SET) == 0)                               //set fp to beginning of file
    {
        while (1)
        {
            fseek(m_boardFile.file, (count - 1) * MAX_MESSAGE_SIZE, SEEK_SET);
            char temp = fgetc(m_boardFile.file);                                //Get char and force EOF to trigger

            if (feof(m_boardFile.file) != 0)
            {
                return count;
            }
            else
            {
                ++count;
                ungetc(temp, m_boardFile.file);                                //Since we didn't find EOF, put char back
            }
        }
    }
    else
        {
            m_boardFile.lastError = UpdateFailed;
            return 0;
        }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: WriteFile

    Accepts a message on stdin and writes message to message file

    @return     -- On failure, lastError is set and 0 returned. On success, returns 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int WriteFile()
{
    char messageHeader[MAX_MESSAGE_SIZE];           //For holding message header
    char userMessage[MAX_MESSAGE_SIZE];             //Buffer to hold user message temporarily to avoid overflow
    char messagePad[MAX_MESSAGE_SIZE];              //For padding messageToWrite to MAX_MESSAGE_SIZE
    char messageToWrite[MAX_MESSAGE_SIZE];          //Message to be written to BB
    int sizeMessageHeader = 0;                     //Size of message
    int sizeUserMessage = 0;                     //Size of message
    int sizeMessagePad = 0;                     //Size of message
    int sizeMessageMinusPad = 0;

    printf("Enter the message to write to the board\n");            //Prompt user for message
    fflush(stdout);
    fgets(userMessage, MAX_MESSAGE_SIZE, stdin);                    //Get user message

    sprintf(messageHeader, "<message n = %d>\n", m_boardFile.nextMessageNumber); //Create message header
    sizeMessageHeader = (int)strlen(messageHeader);
    sizeUserMessage = (int)strlen(userMessage);
    sizeMessageMinusPad = sizeMessageHeader + sizeUserMessage + lengthendXML;  //Set sizeMessageToWrite to sum of lengths of header, userMessage, and footer

    if(sizeMessageMinusPad > MAX_MESSAGE_SIZE)                       //Check for message buffer overflow
    {
        m_boardFile.lastError = WriteMessageBufferOverflow;
        return 0;
    }

    sizeMessagePad = MAX_MESSAGE_SIZE - sizeMessageMinusPad - 1; //Pad the end of message with spaces, save one for '\0'
    sprintf(messagePad, "%*s\n", sizeMessagePad, "");   //length minus 4 to account for three newlines and \0 //TODO what???

    sprintf(messageToWrite, "<message n = %d>", m_boardFile.nextMessageNumber); //Add message header to messageToWrite
    strcat(messageToWrite, messagePad);                         //Add padding to messageToWrite
    strcat(messageToWrite, userMessage);                        //Add user message to messageToWrite
    strcat(messageToWrite, endXml);                             //And message footer to messageToWrite

    OpenFile(m_boardFile.fileName);         //OPEN
    m_boardFile.nextMessageNumber = UpdateFile();                   //Get number of next message

    printf("in write, next message number = %d\n", m_boardFile.nextMessageNumber); //TODO

    fseek(m_boardFile.file, 0, SEEK_END);                       //Update file pointer

    if( fprintf(m_boardFile.file, "%s", messageToWrite) < 0 || m_boardFile.nextMessageNumber < 0 )
    {
        m_boardFile.lastError = WriteFailed;
        return 0;
    }

    fclose(m_boardFile.file); //CLOSE

    return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: ReadFileBySequenceNumber

    Reads and prints a message based on sequence number.

    @return     -- On failure, lastError is set and 0 returned. On success, returns 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int ReadFileBySequenceNumber(int sequenceNumber)
{
    char messageToPrint[MAX_MESSAGE_SIZE];
    char beginXml[MAX_MESSAGE_SIZE];
    char messageToParse[MAX_MESSAGE_SIZE];
    int seekLength = (sequenceNumber - 1) * (MAX_MESSAGE_SIZE - 1); //Sequences 1 starts at line 0, message has '\0'

    OpenFile(m_boardFile.fileName);

    m_boardFile.nextMessageNumber = UpdateFile();                            //Update next message number TODO
    printf("in read, next message number = %d\n", m_boardFile.nextMessageNumber); //TODO

    if(sequenceNumber < m_boardFile.nextMessageNumber && sequenceNumber > 0) //Check sequence number exists and > 0
    {
        if(fseek(m_boardFile.file, seekLength, SEEK_SET) == 0)
        {
            fread(messageToParse, sizeof(char), MAX_MESSAGE_SIZE, m_boardFile.file); //Get string of data to parse

            sprintf(beginXml, "<message n = %d>",  sequenceNumber);

            int xmlResult = XMLParser(beginXml, endXml, messageToParse, messageToPrint, MAX_MESSAGE_SIZE);

            printf("NEXT MESSAGE NUMBER = %d\n", m_boardFile.nextMessageNumber);
            printf("SEQUENCE NUMBER = %d\n", sequenceNumber);

            if(xmlResult == 0)                                              //If invalid message in file
            {
                puts("xml result = 0");
                m_boardFile.lastError = InvalidXMLSyntax;
                fclose(m_boardFile.file);
                return 0;
            }
            else if(xmlResult == -1)    //If message too large
            {
                m_boardFile.lastError = ReadMessageBufferOverflow;
                fclose(m_boardFile.file);
                return 0;
            }
            printf("Message: %s", messageToPrint);
            fclose(m_boardFile.file);
            return 1;
        }
        else                                                               //If seek fails...
        {
            m_boardFile.lastError = SeekFailed;
            fclose(m_boardFile.file);
            return 0;
        }
    }
    else                                                           //Invalid user input or sequence number doesn't exist
    {
        m_boardFile.lastError = ReadFailed;
        fclose(m_boardFile.file);
        return 0;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: PrintSequenceNumbers

    Prints the available sequence numbers found in the file

    @return     -- On failure, lastError is set and 0 returned. On success, returns 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int PrintSequenceNumbers()
{
    OpenFile(m_boardFile.fileName); //OPEN
    m_boardFile.nextMessageNumber = UpdateFile();

    printf("in print seq numbers, next message # = %d\n", m_boardFile.nextMessageNumber); //TODO

    if(m_boardFile.nextMessageNumber == 0)
    {
        m_boardFile.lastError = PrintSequenceFailed;
        fclose(m_boardFile.file);
        return 0;
    }

    int i;
    printf("Sequence Numbers Available:\n");
    for(i = 0; i < m_boardFile.nextMessageNumber-1; ++i)
    {
        printf("%d\n", i+1 );
    }
    fflush(stdout);
    fclose(m_boardFile.file); //CLOSE
    return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: OpenFile

    Opens the bulletin board file for read and write operations

    @return exits on failure
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
FILE * OpenFile(const char * fileName)
{
    FILE * fp = fopen(fileName, "a+");
    if(NULL == fp)
    {
        printf("Failed to open file %s. Exiting...", fileName);
        exit(1);
    }
    return fp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: PrintMenu

    Prints the BB bulletin board menu and resets lastError

    @return     --  Returns 0 when user requests exit, else 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int PrintMenu()
{
    m_boardFile.lastError = NoError; //Reset error
    printf( "\n"
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
    if(ERROR == optionResult)           //If something failed
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
    return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: GetOption

    Gets and parses the user's option.

    @return     --      EXIT if user selects exit, 0 on error
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int GetOption() {
    int userOption = 0, readOption = 0;

    readOption = ParseUserOption(&userOption);

        switch (userOption)
        {
            case WRITE:
                return WriteFile();
            case READ:
                return ReadFileBySequenceNumber(readOption);
            case LIST:
                return PrintSequenceNumbers();
            case EXIT:
                return EXIT;
            case INVALID:
                m_boardFile.lastError = InvalidBoardOption;
                return 0;
            default:
                break;
        }
    return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: InitBBFile

    Initializes the m_boardFile struct and sets the number of messages
    already present in the file we're writing to
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void InitBBFile(const char *filename)
{
    m_boardFile.fileName = filename;
    m_boardFile.lastError = NoError;
    m_boardFile.file = OpenFile(filename);
    m_boardFile.nextMessageNumber = UpdateFile();

    printf("in init, next message# = %d\n", m_boardFile.nextMessageNumber); //TODO

    if(m_boardFile.nextMessageNumber == 0)
    {
        printf("Failed to initialize BB file. Exiting...\n");
        exit(1);
    }
    fclose(m_boardFile.file);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: PrintErrorMessage

    Prints an error message if m_boardFile.lastError is set

    @return void
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
        case NoError:
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
int XMLParser( const char* beginXml, const char* endXml, char* clientMessage, char* token, int tokenSize)
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
        for(i = 1; i < clientMessageLength; ++i) //Check for valid delimiter here
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
            else
            {
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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: EatInputUntilNewLine

    Calls getchar until a newline is found
    @return number of characters consumed
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int EatInputUntilNewline()
{
    int charCount = 0;
    while(getchar() != '\n')//Consume newline
    {
        ++charCount;
    }
    return charCount;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: ParseUserOption

    Parses the user's selection and sets the userOption

    @param userOption  --  The user option to be set
    @return            --  The sequence number to read if read option is selected, else returns 0
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int ParseUserOption(int* userOption)
{
    char userOptionString[MAX_MESSAGE_SIZE];
    memset(userOptionString, 0, MAX_MESSAGE_SIZE);
    fgets(userOptionString, MAX_MESSAGE_SIZE, stdin);   //Get string of data to parse
    userOptionString[strlen(userOptionString)-1] = '\0';  //And null terminate string

    if(strcmp(userOptionString, "write") == 0)
    {
        *userOption = WRITE;
    }
    else if(strcmp(userOptionString, "list") == 0)
    {
        *userOption = LIST;
    }
    else if(strcmp(userOptionString, "exit") == 0)
    {
        *userOption = EXIT;
    }
    else //Else find out if we're reading.
    {
        char readOption[MAX_MESSAGE_SIZE];
        strcpy(readOption, userOptionString);
        readOption[READ_STRING_LENGTH] = '\0';   //Set strlen to 4 for "read"
        if(strcmp(readOption, "read") != 0 || userOptionString[READ_SPACE] != ' ')  //If not read OR if no space is present in the option, e.g. read6
        {
            *userOption = INVALID;
        }
        else
        {
            char* readSequenceNumber = userOptionString+READ_SEQUENCE_NUMBER;
            *userOption = READ;
            return  atoi(readSequenceNumber);
        }
    }
    return 0;
}