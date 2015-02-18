#include <stdio.h>


typedef enum
{
    UpdateFailed,
    WriteFailed,
    ReadFailed,
    PrintSequenceFailed
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
int ReadFile();
int PrintSequenceNumbers();
int OpenFile(char*);
int PrintMenu();
int GetOption();
void PrintErrorMessage();