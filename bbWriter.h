/*
 * bbWriter.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 *
 */

#include <stdio.h>


typedef enum
{
    UpdateFailed,
    WriteFailed,
    ReadFailed,
    PrintSequenceFailed,
    InvalidBoardOption,
    SeekFailed,
    InvalidXMLSyntax,
    ReadMessageBufferOverflow,
    WriteMessageBufferOverflow,
    NoError
}ErrorCode;

typedef struct{
    FILE* file;
    int count;          //Current message counter.
                        //Idea! Send message counter across hosts to check for
                        //file changes
    ErrorCode lastError; //If an error is encountered, this is set
}BBFile;




int UpdateFile();
int WriteFile();
int ReadFileBySequenceNumber();
int PrintSequenceNumbers();
int OpenFile(const char*);
int PrintMenu();
int GetOption();
void PrintErrorMessage();
int XMLParser(const char*, const char*, char*, char*, int);
int InitBBFile();
int EatInputUntilNewline();