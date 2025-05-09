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

  //just used the date of the time when i wrote this
  char aszString[sizeof(serverAccept.acFormattedString)] = {0};
  sprintf(aszString, "127.0.0.1 SMTP %s 09.05.2025 18:03:59", pszId);

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
      char *pszClientGreeting = " Hello sensor";

      //finding where the ip starts, which is after the first 0
      int iIndex = strcspn(clientHelo.acFormattedString, ".") + 1;
      //then copy values into pszClientId
      int i = 0, iClientHeloStart = strlen(clientHelo.acFormattedString) - iIndex;
      while(1){
        pszClientIp[i] = clientHelo.acFormattedString[i + iIndex];
        i++;
        if(i >= sizeof(clientHelo.acFormattedString) - iClientHeloStart){
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

void ReadToFile(FILE *f, int sockFd){
  int iBytesRead = 0, iTotalRead = 0;
  //recieve data and write to file
  //just assuming that the data is 9999 - 1 bytes
  struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *pDataFile = malloc(
  sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE) + 9999);
  int iStructSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE);
 
  //first read the head part
  iBytesRead = recv(sockFd, pDataFile, iStructSize - 1, 0);
  if(iBytesRead < 0){
    printf("Failed to read header for data file\n");
  } else {
    //get size from the header, from datasize to delimeter
    char aszDataSize[5] = {0}; //must be +1 since zero terminator
    memcpy(aszDataSize, pDataFile->stHead.acDataSize, 4);
    aszDataSize[5] = '\0';
    
    int iDataSize = atoi(aszDataSize);
    if(iDataSize == 0){
      printf("Couldnt get data size\n");
    } else {
      //now recv the data with datasize
      while(1){
        //+iTotalBytesRead in case there are multiple recv calls, so it appends
        iBytesRead = recv(sockFd, pDataFile->acFileContent + iTotalRead, iDataSize, 0);
        iTotalRead += iBytesRead;

        if(iBytesRead == 0){
          printf("Finished reading file content from client\n");
          break;
        }
        
        if(iTotalRead >= iDataSize){
          printf("Finished reading file content from client\n");
          break;
        }
      }

      //now that every piece of data is read.. write it to the file
      //crlf crlf is 5 bytes, so just subtract that from the amount to be written
      fwrite(pDataFile->acFileContent, iTotalRead - 5, 1, f);
    }
  }
  
  free(pDataFile);
}

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
      if(iPort != 0){
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
  saAddr.sin_addr.s_addr = htonl(0x7F000001); //hardcoding 127.0.0.1
  
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

  //recieveing mailFrom struct
  struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM mailFrom = {0};
  int iMailFromSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM);
  if(recv(sockNewFd, &mailFrom, iMailFromSize, 0) < 0){
    printf("Failed to recieve MAILFROM\n");
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  
  //chekcing if its a valid email by seeing if it has the @ in it
  //if it is less than the strlen, it contains the @
  if(strcspn(mailFrom.acFormattedString, "@") < strlen(mailFrom.acFormattedString)){
    //it is is valid, send the next ok server reply
    char *pszStatusMailTo = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK;
    char *pszMailToString = "Sender address is OK";
    if(SendServerReply(sockNewFd, header, pszStatusMailTo, pszMailToString, 
    strlen(pszMailToString)) < 0){
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
  } else {
    //send that email addres was not okay, and that the server is closing
    char *pszStatusMailTo = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_CLOSED;
    char *pszMailToString = "Sender address is invalid, closing server";
    if(SendServerReply(sockNewFd, header, pszStatusMailTo, pszMailToString, 
    strlen(pszMailToString)) < 0){
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
    
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  
  //recieve rcptTo struct
  struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO rcptTo = {0};
  int iRcptToSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO);
  if(recv(sockNewFd, &rcptTo, iRcptToSize, 0) < 0){
    printf("Failed to recieve RCPTTO\n");
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }

  //check if rcptTo recieveing email is okay the same way as sender email
  if(strcspn(rcptTo.acFormattedString, "@") < strlen(rcptTo.acFormattedString)){
    char *pszStatusRcptTo = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK;
    char *pszRcptToString = "Recieving address is OK";
    if(SendServerReply(sockNewFd, header, pszStatusRcptTo, pszRcptToString, 
    strlen(pszRcptToString)) < 0){
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
  } else {
    char *pszStatusRcptTo = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_CLOSED;
    char *pszRcptToString = "Recieving address is invalid, closing server";
    if(SendServerReply(sockNewFd, header, pszStatusRcptTo, pszRcptToString, 
    strlen(pszRcptToString)) < 0){
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
    
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  
  //retrieve filename to open this file and write to it
  struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD dataCmd = {0};
  int iDataCmdSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD);
  if(recv(sockNewFd, &dataCmd, iDataCmdSize, 0) < 0){
    printf("Failed to recieve CLIENTDATACMD\n");
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  }
  char aszFileName[sizeof(dataCmd.acFormattedString)] = {0};
  memcpy(aszFileName, dataCmd.acFormattedString, sizeof(aszFileName));
  
  //check if filename is valid by opening the file
  FILE *f = fopen(aszFileName, "w");
  if(f == NULL){
    printf("Failed to open file, might be due to invalid filename\n");
  
    //send a struct that says it failed
    char *pszStatusDataCmd = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_CLOSED;
    char *pszDataCmdString = "Invalid filename";
    SendServerReply(sockNewFd, header, pszStatusDataCmd, pszDataCmdString, 
    strlen(pszDataCmdString));
  
    fclose(f);
    close(sockNewFd); sockNewFd = -1;
    close(sockFd); sockFd = -1;
    return 1;
  } else {
    //send struct that says server is ready for data
    char *pszStatusDataCmd = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_READY;
    char *pszDataCmdString = "Filename is Okay";
    if(SendServerReply(sockNewFd, header, pszStatusDataCmd, pszDataCmdString, 
    strlen(pszDataCmdString)) < 0){
      fclose(f);
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
  }
  ReadToFile(f, sockNewFd);
  fclose(f); 

  //send next struct that says OK  
  char *pszStatusNext = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK;
  char *pszNextMessage = "Read and wrote to file\n";
  if(SendServerReply(sockNewFd, header, pszStatusNext, pszNextMessage, 
  strlen(pszNextMessage)) >= 0){
  
    //recieve quit message
    struct EWA_EXAM25_TASK5_PROTOCOL_CLOSECOMMAND quitCmd = {0};
    int iQuitSize = sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLOSECOMMAND);
    if(recv(sockNewFd, &quitCmd, iQuitSize, 0) < 0){
      printf("Failed to read QUIT command\n");
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
    
    //then send server is closing
    char *pszStatusQuit = EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_CLOSED;
    char *pszQuitMessage = "Server is closing";
    if(SendServerReply(sockNewFd, header, pszStatusQuit, pszQuitMessage, 
    strlen(pszQuitMessage)) < 0){
      close(sockNewFd); sockNewFd = -1;
      close(sockFd); sockFd = -1;
      return 1;
    }
  }

  //cleanup and exit program
  close(sockNewFd);
  sockNewFd = -1;
  close(sockFd);
  sockFd = -1;
  
  return 0;
}

