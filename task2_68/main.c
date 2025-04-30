#include "include/count.h"
#include "include/hash.h"
#include "include/sum.h"
#include "include/main.h"

int main(int iArgC, char *apszArgV[]){
  FILE *fInput = fopen("pgexam25_test.txt", "r");
  if(fInput == NULL){
    printf("Failed to open input file\n");
    return 1;
  }
  
  
  
  FILE *fOutput = fopen("pgexam25_output.bin", "wb");
  if(fOutput == NULL){
    printf("Failed to open output file\n");
    return 1;
  }
  
  
  fclose(fInput);
  fclose(fOutput);

  return 0;
}
