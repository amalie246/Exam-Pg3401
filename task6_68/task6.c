#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "include/task6.h"

//run with ./task6 -server [server] -port [port]

//Original code by David Wheeler and Roger Needham
//Source: https://www.schneier.com/wp-content/uploads/2015/03/TEA-2.c
//Changes made: changed from long to int
void decrypt(unsigned int *const v, unsigned int *const w, const unsigned int *const k){
  register unsigned int y=v[0], z=v[1], sum=0xC6EF3720, delta=0x9E3779B9, a=k[0],
  b=k[1], c=k[2], d=k[3], n=32;
  
  while(n-->0){
    z -= (y<<4)+c ^ y+sum ^ (y>>5)+d;
    y -= (z<<4)+a ^ z+sum ^ (z>>5)+b;
    sum -= delta;
  }
  
  w[0]=y; w[1]=z;
}

//this function should work as an allow list for characters, so that
//the gibberish is not printed out and decrypted...
int CheckChar(int iC){
  int iRc = ERROR;
  //i just found the decimal ascii values from an ascii table

  //lower and upper bound for printable characters, https://www.ascii-code.com
  int iLowAscii = 32, iHighAscii = 127;
  //maybe also need other chars like \r, \n, \t just in case
  int iCr = 13, iLf = 10, iTab = 9;

  //then check if it is either the special chars or in the printable
  if(iC >= iLowAscii && iC <= iHighAscii){
    iRc = OK;
  }
  
  if(iC == iCr || iC == iLf || iC == iTab){
    iRc = OK;
  }

  return iRc;
}

//decrypts the unsigned ints, 64-bits at a time, checks if all decrypted
//characters are okay, if so then write into pszDecrypted
int DecryptText(unsigned int *puiText, unsigned int uiKey[4], int iSize, 
char *pszDecrypted){
  int iRc = ERROR, iTotalBytes = 0;
  for(int i = 0; i < iSize; i += 2){
    unsigned int uiV[2] = {0};
    uiV[0] = puiText[i];
    uiV[1] = puiText[i + 1];
    
    unsigned int uiW[2] = {0};
    
    decrypt(uiV, uiW, uiKey);
    char cRes[9] = {0};
    memcpy(cRes, uiW, 8);
    
    //check if all 8 bytes are printable, if not, no need to continue decrypting
    for(int j = 0; j < 8; j++){
      if(CheckChar(cRes[j]) == ERROR){
        return iRc;
      }
    }
    iRc = OK;
    //if it reaches here, it means we have a possible good decrypt
    printf("%s", cRes);

    //return it back to the pszDecrypted
    memcpy(pszDecrypted + iTotalBytes, cRes, 8);
    iTotalBytes += 8;
  }
  printf("\n");
  return iRc;
}

//loops through all combinations for 0x00 to 0xFF and also creates the repeating
//pattern that the key needs to have
void BruteForce(unsigned int *puiText, int iSize, char *pszDecrypted){
  for(int i = 0; i < 256; i++){
    unsigned int uiKey[4] = {0};

    char acHex[8] = {0};
    sprintf(acHex, "%02X%02X%02X%02X", i, i, i, i);
    //make sure that it actually is treated like hex so i use strtol
    unsigned int iHex = strtol(acHex, NULL, 16);
    
    uiKey[0] = iHex;
    uiKey[1] = iHex;
    uiKey[2] = iHex;
    uiKey[3] = iHex;

    //print key if its a good decrypt
    if(DecryptText(puiText, uiKey, iSize, pszDecrypted) != ERROR){
      printf("\nKEY: %X %X %X %X\n", uiKey[0], uiKey[1], uiKey[2], uiKey[3]);
    }
  }
}


//for setting up the client, i reused code from exam preparations task 3
//with the client.c code, same as for task 5.
//PG3401_Exercises_09-12_exam_preparation.pdf
int main(int iArgC, char *apszArgV[]){
  char aszIpAddr[16] = {0}; //think this is max size for an ip address
  int iPort = 0, iRc = ERROR;
  
  if(iArgC == 5){
    //check if flags are correct
    int iServerCmp = strcmp(apszArgV[1], "-server");
    int iPortCmp = strcmp(apszArgV[3], "-port");
    
    if(iServerCmp + iPortCmp == 0){
      iPort = atoi(apszArgV[4]);
      
      if(iPort != 0){
        //find out what size to copy
        if(sizeof(aszIpAddr) < strlen(apszArgV[2])){
          memcpy(aszIpAddr, apszArgV[2], sizeof(aszIpAddr) - 1);
        } else {
          memcpy(aszIpAddr, apszArgV[2],strlen(apszArgV[2]));
        }
        iRc = OK;
        aszIpAddr[15] = '\0';
      } else {
        printf("Invalid port number\n");
      } //invalid port
    } else {
      printf("Invalid flags, use -server and -port\n");
    } //invalid flags
  } else {
    printf("Invalid amount of args\n");
  } //invalid amount
  
  if(iRc != OK){
    return 1;
  }
  
  struct sockaddr_in saAddr = {0};
  int sockFd = 0;

  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFd < 0){
    printf("Socket failed\n");
    return 1;
  }
  
  saAddr.sin_family = AF_INET;
  saAddr.sin_port = htons(iPort);
  //can hardcode the ip address like in task 5, but since ip address is an argument
  //i do it dynamically like this instead
  saAddr.sin_addr.s_addr = inet_addr(aszIpAddr);
  
  if(connect(sockFd, (struct sockaddr*)&saAddr, sizeof(saAddr)) < 0 ){
    printf("Connect failed\n");
    close(sockFd); sockFd = -1;
    return 1;
  }

  //read it in unsigned chars in case char is signed and can be negative
  unsigned char acBuf[4096] = {0};
  int iTotalRead = 0;
  while(1){
    int iRead = recv(sockFd, acBuf + iTotalRead, 4096 - iTotalRead, 0);
    if(iRead < 0){
      printf("Failed to read contents\n");
      close(sockFd); sockFd = -1;
      return 1;
    }
    iTotalRead += iRead;
    if(iRead == 0) break;
  }
  close(sockFd);
  sockFd = -1;

  //find content length aka size of body
  unsigned char *pszContentLength = strstr(acBuf, "Content-Length: ");
  pszContentLength += 16; //length of "Content-Length: ", i counted it
  char acContentLength[4] = {0};
  memcpy(acContentLength, pszContentLength, 3);
  acContentLength[3] = '\0';
  int iSize = atoi(acContentLength);
  
  //strstr finds substring, body is after \r\n\r\n in HTTP/1.1
  unsigned char *pszBody = strstr(acBuf, "\r\n\r\n");
  pszBody += 4; //\r\n\r\n has length of 4, so move pointer 4 ahead
  
  //copy 4 bytes at a time into unsigned ints
  unsigned int *puiV = malloc(iSize);
  if(puiV == NULL){
    printf("Malloc failed!\n");
    return 1;
  }
  //assuming iSize is divisible by 4 since the encrypted text is padded
  for(int i = 0; i < (iSize / 4); i++){
    memcpy(&puiV[i], pszBody, 4);
    pszBody += 4;
  }

  char *pcDecrypted = malloc(iSize);
  if(pcDecrypted == NULL){
    printf("Malloc failed!\n");
    free(puiV);
    puiV = NULL;
    return 1;
  }


  BruteForce(puiV, (iSize / 4), pcDecrypted);

  free(puiV);
  puiV = NULL;

  //when it is decrypted, create a file and write the decrypted values
  FILE *f = fopen("decrypted.txt", "w");
  if(f == NULL){
    printf("Failed to open file\n");
    return 1;
  }
  
  fwrite(pcDecrypted, strlen(pcDecrypted), 1, f);

  free(pcDecrypted);
  pcDecrypted = NULL;
  fclose(f);
  return 0;
}




