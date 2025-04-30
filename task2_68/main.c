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

  unsigned int iHash;
  Task2_SimpleDjb2Hash(fInput, &iHash);
  memcpy(md->byHash, &iHash, 4);

    
  int iSizeOfFile = 0, iSumOfChars = 0;
  Task2_SizeAndSumOfCharacters(fInput, &iSizeOfFile, &iSumOfChars);
  if(iSizeOfFile == 0 || iSumOfChars == 0){
    printf("Something went wrong in SizeAndSumOfCharacters\n");
    return ERROR;
  }
  md->iFileSize = iSizeOfFile;
  md->iSumOfChars = iSumOfChars;

  char aAlphaCount[26] = {0};
  Task2_CountEachCharacter(fInput, aAlphaCount);
  memcpy(md->aAlphaCount, aAlphaCount, 26);

  //writing to the output file
  FILE *fOutput = fopen("pgexam25_output.bin", "wb");
  if(fOutput == NULL){
    printf("Failed to open output file\n");
    return ERROR;
  }

  printf("Filename: %s\n", md->szFileName);
  printf("File size: %d\n", md->iFileSize);
  printf("Hash: %u\n", md->byHash);
  printf("Sum: %d\n", md->iSumOfChars);
  printf("AlphaCount: ");
  for(int i = 0; i < 26; i++){
    printf("%d, ", md->aAlphaCount[i]);
  }
  printf("\n");

  fwrite(&md, sizeof(struct TASK2_FILE_METADATA), 1, fOutput);

  free(md);
  fclose(fInput);
  fclose(fOutput);

  return OK;
}




