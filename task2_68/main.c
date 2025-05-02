#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/hash.h"
#include "include/sum.h"
#include "include/count.h"
#include "include/main.h"

int main(int iArgC, char *apszArgV[]){
  FILE *fInput = fopen(FILENAME, "r");
  if(fInput == NULL){
    printf("Failed to open input file\n");
    return ERROR;
  }
  
  //md stands for metadata
  struct TASK2_FILE_METADATA *md = malloc(sizeof(struct TASK2_FILE_METADATA));
  if(md == NULL){
    printf("Malloc failed\n");
    return ERROR;
  }
  
  //setting filename (can be found in the header file, FILENAME)
  strncpy(md->szFileName, FILENAME, 32);

  unsigned int iHash = 0; //initialized it as 0 for no garbage values
  Task2_SimpleDjb2Hash(fInput, &iHash);
  memcpy(md->byHash, &iHash, 4);

    
  int iSizeOfFile = 0, iSumOfChars = 0; //again no garbage
  Task2_SizeAndSumOfCharacters(fInput, &iSizeOfFile, &iSumOfChars);
  //checking if the values are 0 for error handling, because i would say its safe to
  //assume that they should not be 0
  if(iSizeOfFile == 0 || iSumOfChars == 0){ 
    printf("Something went wrong in SizeAndSumOfCharacters\n");
    return ERROR;
  }
  md->iFileSize = iSizeOfFile;
  md->iSumOfChars = iSumOfChars;

  char aAlphaCount[26] = {0}; //array is now 26 "0"s
  Task2_CountEachCharacter(fInput, aAlphaCount);
  memcpy(md->aAlphaCount, aAlphaCount, 26);

  //writing to the output file
  FILE *fOutput = fopen("pgexam25_output.bin", "wb");
  if(fOutput == NULL){
    printf("Failed to open output file\n");
    return ERROR;
  }

  fwrite(md, sizeof(struct TASK2_FILE_METADATA), 1, fOutput);

  free(md);
  md = NULL; //no dangling pointers
  fclose(fInput);
  fclose(fOutput);

  return OK;
}




