/*
 * bbWriter.h
 * Systems and Networks II
 * Project 3
 * Christopher Schneider & Brett Rowberry
 */

#include "../common.h"


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
    const char * fileName;
    int nextMessageNumber;          //Current message counter.
    ErrorCode lastError;            //If an error is encountered, this is set
}BBFile;

int UpdateFile();
int WriteFile();
int ReadFileBySequenceNumber(int);
int PrintSequenceNumbers();
FILE * OpenFile(const char*);
int PrintMenu();
int GetOption();
void PrintErrorMessage();
int XMLParser(const char*, const char*, char*, char*, int);
void InitBBFile(const char *);
int EatInputUntilNewline();
int ParseUserOption(int*);