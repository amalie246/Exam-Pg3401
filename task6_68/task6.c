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
  register unsigned int y=v[0], z=v[1], sum=0xC6EF3720, delta=0x9E779B9, a=k[0],
  b=k[1], c=k[2], d=k[3], n=32;
  
  while(n-->0){
    z -= (y<<4)+c ^ y+sum ^ (y>>5)+d;
    y -= (z<<4)+a ^ z+sum ^ (z>>5)+b;
    sum -= delta;
  }
  
  w[0]=y; w[1]=z;
}
int CheckChar(char c){

}

void DecryptText(char *pcEncryptedText, unsigned int uiKey[4]){
  int iTotalBytes = 0, iBufferIndex = 0, iBytesToRead = strlen(pcEncryptedText);
  
  char acDecrypted[4096] = {0};
  while(1){
    char cBuf[9] = {0}; //8 bytes = 64 bit
    int iBytesRead = 0;
    
    //copy only 8 bytes into the buffer, since it is pkcs5 padded, i think
    //that the full text has a size divisible by 8
    memcpy(cBuf, pcEncryptedText + iBufferIndex, 8);
    iBufferIndex += 8;
    iTotalBytes += 8;
  
    //convert cBuf to an unsigned int
    unsigned int uiV[2] = {0};
    memcpy(&uiV[0], cBuf, 4);
    memcpy(&uiV[1], cBuf + 4, 4);
    cBuf[9] = '\0';
    
    //where the decrypted text should go
    unsigned int uiW[2] = {0};
      
    //for each key combination, try to decipher
    decrypt(uiV, uiW, uiKey);
      
    char acDecrypted[8] = {0};
    memcpy(acDecrypted, uiW, 8);
      
    printf("%s", acDecrypted);
    
    if(iTotalBytes >= iBytesToRead){
      break;
    }
  }
  printf("\n");
}

void BruteForce(char *pcEncryptedText){
  for(int i = 0; i < 256; i++){
    unsigned int uiKey[4] = {0};

    char acHex[8] = {0};
    sprintf(acHex, "%02X%02X%02X%02X", i, i, i, i);
    int iHex = strtol(acHex, NULL, 16);
    
    uiKey[0] = iHex;
    uiKey[1] = iHex;
    uiKey[2] = iHex;
    uiKey[3] = iHex;

    printf("\nKEY: %X %X %X %X\n", uiKey[0], uiKey[1], uiKey[2], uiKey[3]);
    DecryptText(pcEncryptedText, uiKey);
  }
}


//for setting up the client, i reused code from exam preparations task 3
//with the client.c code, same as for task 5.
//PG3401_Exercises_09-12_exam_preparation.pdf
int main(int iArgC, char *apszArgV[]){
  /*char aszIpAddr[16] = {0}; //think this is max size for an ip address
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
  
  printf("Successfully connected\n");
  
  //recieve a file, TEA encrypted
  char acBuf[BUFFER_SIZE] = {0};
  if(recv(sockFd, acBuf, sizeof(acBuf), 0) < 0){
    printf("Recieve failed\n");
    close(sockFd); sockFd = -1;
    return 1;
  }
  //with this, I found that the protocol is HTTP/1.1, as used in previous exams (2021)
  printf("Recieved: %s\n", acBuf);
  
  //done with socket, close it
  close(sockFd);
  sockFd = -1;
  
  //find what matched \r\n\r\n (\r\n\r\n because it is HTTP/1.1)
  char *pcCRLF = "\r\n\r\n";
  //strstr returns pointer to the match inside acBuf
  char *pcEncryptedText = strstr(acBuf, pcCRLF);
  if(pcEncryptedText == NULL){
    printf("Couldnt find a string match\n");
    return 1;
  }
  //move 4 ahead, because \r\n\r\n is 4 bytes
  pcEncryptedText += 4;

  printf("\nENCRYPTED TEXT: %s\n", pcEncryptedText);
  //try keys and decrypt*/

  //trying a test string:
  char *pcEncryptedText = "FFFFFFB8FFFFFFE061FFFFFFE2FFFFFF8865FFFFFFE371";
  BruteForce(pcEncryptedText);
  
  //when it is decrypted, create a file and write the decrypted values

  return 0;
}
