#include <stdio.h>


typedef struct{
    FILE* file;
    int count; //Current message counter.
                //Idea! Send message counter across hosts to check for
                //file changes
}BBFile;


int UpdateFile();
int WriteFile();
int ReadFile(int);
int PrintSequenceNumbers();
int OpenFile(char*);
void PrintMenu();