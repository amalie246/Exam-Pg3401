#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "include/task6.h"

//run with ./task6 -server [server] -port [port]

/*TODO allow list of all letters and most used chars (. , ! ? etc)*/

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
  
  if(iC == 13 || iC == iLf || iC == iTab){
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
    cRes[8] = '\0';
    
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
 
 //copied pszHeader from the terminal, added \r\n after each line and \r\n\r\n at end
  /*char *pszHeader = "HTTP/1.1 200 OK\r\nConnection close\r\nDate: Mon, May 12 2025 13:40:51\r\nServer: Eastwill Assistant - EXAM 2025\r\nContent-Length: 648\r\nContent-Type: text/encrypted\r\n\r\n";
  int iHeaderSize = strlen(pszHeader);  //HEADER SIZE IS 161
  printf("Header size: %d\n", iHeaderSize);*/
  
  //i know its bad but i couldnt figure out how to do it properly, so i found
  //out that the length of the header is 161 chars with the commented out code above
  char acHeader[162] = {0};
  if(recv(sockFd, acHeader, 161, 0) < 0){
    printf("Recv failed\n");
    close(sockFd); sockFd = -1;
    return 1;
  }
  acHeader[161] = '\0';
  printf("%s\n", acHeader);

  //recieving 162 unisgned ints because 648 / 4 (sizeof unsigned int) = 162
  unsigned int uiV[162];
  if(recv(sockFd, uiV, 648, 0) < 0){
    printf("Failed to recieve binary file\n");
    close(sockFd); sockFd = -1;
    return 1;
  }
  
  //finished with socket, so just closing it
  close(sockFd);
  sockFd = -1;
  
  //here should the decrypted text go
  char acDecrypted[648] = {0};
  BruteForce(uiV, 162, acDecrypted);
  
  //when it is decrypted, create a file and write the decrypted values
  FILE *f = fopen("decrypted.txt", "w");
  if(f == NULL){
    printf("Failed to open file\n");
    return 1;
  }
  
  fwrite(acDecrypted, strlen(acDecrypted), 1, f);

  fclose(f);
  return 0;
}
