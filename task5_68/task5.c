#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include "include/ewpdef.h"

//run with ./task5 -port [port] -id [id]

//tried to make a more reusable function for less redundant code
int SendServerReply(int sockFd, struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER header, char *pszStatusCode, char *pszFormattedString, int iMessageSize){
  int iRc = 0; 
  
  struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY serverReply = {0};
  int iServerReplySize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY);
  
  serverReply.stHead = header;
  
  memcpy(serverReply.acStatusCode, pszStatusCode, 
  sizeof(serverReply.acStatusCode));
  
  serverReply.acHardSpace[0] = 0x20;
  
  //figure out what size to copy so there is no overflow
  if(iMessageSize > sizeof(serverReply.acFormattedString)){
    memcpy(serverReply.acFormattedString, pszFormattedString, 
    sizeof(serverReply.acFormattedString));
  } else {
    memcpy(serverReply.acFormattedString, pszFormattedString, iMessageSize);
  }
  serverReply.acHardZero[0] = 0x00;
  
  //send it to server
  iRc = send(sockFd, &serverReply, iServerReplySize, 0);
  if(iRc < 0){
    printf("Failed to send serverReply (%s)\n", serverReply.acFormattedString);
  }
  
  return iRc;
}

//so not too much stuff is happening in main, i created this EstablishConnection func
//which does server accept, client helo, and then server helo (like a handshake)
int EstablishConnection(int sockFd, struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER header, char *pszId){
  int iRc = 1;
  //starting with server accept
  struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT serverAccept = {0};
  serverAccept.stHead = header;
  memcpy(serverAccept.acStatusCode, "220", sizeof(serverAccept.acStatusCode));
  serverAccept.acHardSpace[0] = 0x20;

  //TODO source stackoverflow (ADD A LINK HERE!!!!!!!!!!!!!!!!!)
  time_t tTime = time(NULL);
  struct tm tm = *localtime(&tTime);//day month year hour min sec
  snprintf(serverAccept.acFormattedString, sizeof(serverAccept.acFormattedString), 
  "127.0.0.1 SMTP %s %02d %02d %d %02d %02d %02d", pszId, tm.tm_mday, tm.tm_mon + 1, 
  tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

  serverAccept.acHardZero[0] = 0x00;

  if(send(sockFd, &serverAccept, 64, 0) >= 0){
    //serverAccept is sent, can now recieve clienthelo
    struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO clientHelo = {0};
    int iHeloSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO);
    
    if(recv(sockFd, &clientHelo, iHeloSize, 0) >= 0){
      //clientHelo is recieved, send client accept then return
      iHeloSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO);
      struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO serverHelo = {0};
      serverHelo.stHead = header;
      memcpy(serverHelo.acStatusCode, EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK, 
      sizeof(serverHelo.acStatusCode));
      serverHelo.acHardSpace[0] = 0x20;
      //Extracting the clients IP address from clientHelo
      char *pszClientIp = malloc(sizeof(serverHelo.acFormattedString));
      char *pszClientGreeting = "Hello sensor";

      //finding where the ip starts, which is after the first 0
      int iIndex = strcspn(clientHelo.acFormattedString, ".");
      //then copy values into pszClientId
      int i = 0, iClientHeloStart = strlen(clientHelo.acFormattedString) - iIndex;
      while(1){
        pszClientIp[i] = clientHelo.acFormattedString[i + iIndex];
        i++;
        if(i >= strlen(clientHelo.acFormattedString) - iClientHeloStart || 
        i >= sizeof(pszClientIp)){
          break;
        }
      }
      //concatenating cliend ip and the greeting message
      strcat(pszClientIp, pszClientGreeting);
      //since pszClientIp is zero terminated, im copying 1 less than the strlen
      //this will not result in overflow because size of pszClientId is already    controlled
      memcpy(serverHelo.acFormattedString, pszClientIp, strlen(pszClientIp) - 1);
      free(pszClientIp);
      pszClientIp = NULL;
      serverHelo.acHardZero[0] = 0x00;
  
      if(send(sockFd, &serverHelo, iHeloSize, 0) >= 0){
        iRc = 0;
      } else {
        printf("Failed to send SERVERHELO\n");
      }
    } else {
      printf("Failed to recieve CLIENTHELO\n");
    }
  } else {
    printf("Failed to send SERVERACCEPT\n");
  }
  
  return iRc;
}

void ReadFile(){}

int main(int iArgC, char *apszArgV[]){
  struct sockaddr_in saAddr = {0}; //for binding
  struct sockaddr_in saClientConnection = {0}; //for accepting
  int iOk = 0, iPort = 0, iReadValue = 0, iAddrLen = sizeof(saAddr);
  int sockFd = 0, sockNewFd = 0;
  char aszId[32] = {0};
  
  //check if program was started correctly
  if(iArgC == 5){
    int iPortCmp = strcmp(apszArgV[1], "-port");
    int iIdCmp = strcmp(apszArgV[3], "-id");
    
    if(iPortCmp + iIdCmp == 0){
      
      iPort = atoi(apszArgV[2]);
      if(iPort != 0 /*&& strlen(apszArgV[2]) < 32*/){
        memcpy(aszId, apszArgV[4], strlen(apszArgV[4]));
        aszId[strlen(apszArgV[4]) - 1] == '\0';
      } else {
        printf("Invalid input for -port or -id\n");
        iOk = 1;
      } //invalid args (value)
    } else {
      printf("Invalid flags, run with -port and -id\n");
      iOk = 1;
    } //invalid args (flags)
  } else {
    printf("Invalid amount of args. Run with ./task5 -port [port] -id [id]\n");
    iOk = 1;
  } //invalid amount of args

  if(iOk != 0){
    //exit program because the user did not start it properly
    return 1;
  }
  
  //create the server
  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFd < 0){
    printf("Failed to create socket\n");
    return 1;
  }
  
  saAddr.sin_family = AF_INET;
  saAddr.sin_port = htons(iPort);
  saAddr.sin_addr.s_addr = htonl(0x7F000001); //hardcodin 127.0.0.1
  
  if(bind(sockFd, (struct sockaddr*)&saAddr, sizeof(saAddr)) < 0){
    printf("Bind failed\n");
    return 1;
  }
  listen(sockFd, 5);
  
  sockNewFd = accept(sockFd, 
  (struct sockaddr*)&saClientConnection, (socklen_t*)&iAddrLen);
  if(sockNewFd < 0){
    printf("Accept failed\n");
    return 1;
  } 

  //create the header first, since it is pretty much reusable for the rest of the task
  struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER header = {0};
  memcpy(header.acMagicNumber, 
  EWA_EXAM25_TASK5_PROTOCOL_MAGIC, sizeof(header.acMagicNumber));

  //snprintf zero terminates so i am using a temporary buffer with 1 more size
  char aszTemp[5] = {0};
  snprintf(aszTemp, 5, "%04d", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
  memcpy(header.acDataSize, aszTemp, sizeof(aszTemp) - 1);
  header.acDelimeter[0] = '|';
  
  //then, establish connection (accept, helo from client and then helo from server)
  if(EstablishConnection(sockNewFd, header, aszId) == 1){
    //no need to continue if connection wasnt established.. cleanup and return
    printf("Failed to establish connection\n");
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }

  
  struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM mailFrom = {0};
  int iMailFromSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM);
  if(recv(sockNewFd, &mailFrom, iMailFromSize, 0) < 0){
    printf("Failed to recieve MAILFROM\n");
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  
  //check if mailFrom was okay
  char *pszStatusCode = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK;
  char *pszFormattedString = "Sender address is OK";
  if(SendServerReply(sockNewFd, header, pszStatusCode, pszFormattedString, 
  strlen(pszFormattedString)) < 0){
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  

  close(sockNewFd);
  sockNewFd = -1;
  close(sockFd);
  sockFd = -1;
  
  return 0;
}

