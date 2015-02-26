/*
 * bbWriter.c
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#include "bbWriter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
 const int lengthendXML = 11;
 const char * endXml = "</message>\n";
 const char * TEST_ME = "APPEND";
 const int READ_STRING_LENGTH = 4;          //Length of read string
 const int READ_SPACE = 4;                  //Array index of where we expect a space to be
 const int READ_SEQUENCE_NUMBER = 5;        //Array index of where we expect the sequence number to be
 const int SPECIAL_CHARACTER_COUNT = 4;     //Number of special characters in message

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: main

    Accepts a file in argv[1]

    @param argc         --  number of args
    @param argv         --  arg vector
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 0;
    }

    if(0 == OpenFile(argv[1])) //Try to open file
    {
        printf("Failed to open file %s\n", argv[1]);
        return 0;
    }
    if(InitBBFile() == 0) //Initialize BB File struct
    {
        PrintErrorMessage(); //This only fails if UpdateFile() fails
        fflush(stdout);
        return 0;
    }

    while(PrintMenu());
//    fclose(m_boardFile.file); //Close file

    return 0;
}

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
                ungetc(temp, m_boardFile.file);                                 //Since we didn't find EOF, put char back
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
    char messageToWrite[MAX_MESSAGE_SIZE];          //Message to be written to BB
    char userMessage[MAX_MESSAGE_SIZE];             //Buffer to hold user message temporarily to avoid overflow
    char messagePad[MAX_MESSAGE_SIZE];              //For padding messageToWrite to MAX_MESSAGE_SIZE
    char messageHeader[MAX_MESSAGE_SIZE];           //For holding message header
    int messageSize = 0;                            //Size of message

    memset(messageToWrite, 0, MAX_MESSAGE_SIZE);    //Clear out arrays
    memset(userMessage, 0, MAX_MESSAGE_SIZE);
    memset(messagePad, 0, MAX_MESSAGE_SIZE);

    printf("Enter the message to write to the board\n");            //Prompt user for message
    fflush(stdout);
    fgets(userMessage, MAX_MESSAGE_SIZE, stdin);             //Get user message

    m_boardFile.nextMessageNumber = UpdateFile();           //Get number of next message

    sprintf(messageHeader, "<message n = %d>\n", m_boardFile.nextMessageNumber); //Create message header
    messageSize = (int)strlen(messageHeader) + (int)strlen(userMessage) + lengthendXML;  //Set messageSize to sum of lengths of header, userMessage, and footer

    if(messageSize > MAX_MESSAGE_SIZE)                       //Check for message buffer overflow
    {
        m_boardFile.lastError = WriteMessageBufferOverflow;
        return 0;
    }

    int messagePaddingLength = MAX_MESSAGE_SIZE - messageSize - lengthendXML - 1  ; //Pad the end of message with spaces
    sprintf(messagePad, "%*s\n", messagePaddingLength, "");   //length minus 4 to account for three newlines and \0

    sprintf(messageToWrite, "<message n = %d>", m_boardFile.nextMessageNumber); //Add message header to messageToWrite
    strcat(messageToWrite, messagePad);                         //Add padding to messageToWrite
    strcat(messageToWrite, userMessage);                        //Add user message to messageToWrite
    strcat(messageToWrite, endXml);                             //And message footer to messageToWrite

    fseek(m_boardFile.file, 0, SEEK_END);                       //Update file pointer

    if( fprintf(m_boardFile.file, "%s", messageToWrite) < 0 || m_boardFile.nextMessageNumber < 0 )
    {
        m_boardFile.lastError = WriteFailed;
        return 0;
    }
    else
    {
        return 1;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: ReadFileBySequenceNumber

    Reads and prints a message based on sequence number.

    @return     -- On failure, lastError is set and 0 returned. On success, returns 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int ReadFileBySequenceNumber(int sequenceNumber)
{
    m_boardFile.nextMessageNumber = UpdateFile();
    if(sequenceNumber < m_boardFile.nextMessageNumber      //Check sequence number exists
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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: PrintSequenceNumbers

    Prints the available sequence numbers found in the file

    @return     -- On failure, lastError is set and 0 returned. On success, returns 1
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int PrintSequenceNumbers()
{
    m_boardFile.nextMessageNumber = UpdateFile();
    if(m_boardFile.nextMessageNumber == 0)
    {
        m_boardFile.lastError = PrintSequenceFailed;
        return 0;
    }

    int i;
    printf("Sequence Numbers Available:\n");
    for(i = 0; i < m_boardFile.nextMessageNumber-1; ++i)
    {
        printf("%d\n",i+1 );
    }
    fflush(stdout);
    return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  FUNCTION: OpenFile

    Opens the bulletin board file

    @return 0 on failure, 1 on success
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Opens file for R/W operations
//Returns 1 on success, 0 on failure
int OpenFile(const char* fileName)
{
    m_boardFile.file = fopen(fileName, "a+");
    if(NULL == m_boardFile.file) return 0;
    return 1;
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

    @return     --  Number of messages in file or 0 if failure
 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int InitBBFile()
{
    m_boardFile.lastError = NoError;
    m_boardFile.nextMessageNumber = UpdateFile();
    return m_boardFile.nextMessageNumber;
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