#include <stdio.h>
#include <string.h>
#include "include/doublelinkedlist.h"

#define CONTINUE 0
#define FINISHED 1

//TODO improve readability
//TODO fix better structure
//TODO finish tasks

void PrintMenu();
int HandleMenu(FLIGHT_DEPARTURE **ppHead);
void PrintList(FLIGHT_DEPARTURE **ppHead);
int ReadInput(char *pszDestination, int iSize);
void PrintPassengers(FLIGHT_DEPARTURE *pFlight);

int main(int iArgC, char *apszArgV[]){
  int iComplete = CONTINUE;
  FLIGHT_DEPARTURE *pHead = {0}; 
  AddFlight(&pHead, "AA-01", "Italy", 140, 2015);
  AddPassenger(pHead, "Amalie", 21, 2);
  AddPassenger(pHead, "Nilsen", 21, 1);

  AddFlight(&pHead, "AA-02", "France", 180, 1205);
  FLIGHT_DEPARTURE *p1 = GetFlightById(&pHead, "AA-02");
  AddPassenger(p1, "Sondre", 24, 1);
  AddPassenger(p1, "Somdal", 25, 10);

  AddFlight(&pHead, "AA-03", "Greece", 89, 1335);
  FLIGHT_DEPARTURE *p2 = GetFlightById(&pHead, "AA-03");
  AddPassenger(p2, "Oda", 21, 10);
  AddPassenger(p2, "Bergvoll", 22, 9);
  printf("Before deleting 1. element\n");
  PrintList(&pHead);
  DeleteFlight(&pHead, pHead);
  printf("After deleting 1. element: \n");
  PrintList(&pHead);

  printf("Before deleting last element: \n");
  PrintList(&pHead);
  printf("After deleting last element\n");
  DeleteFlight(&pHead, p2);
  PrintList(&pHead);
  /*while(iComplete != FINISHED){
    PrintMenu();
    iComplete = HandleMenu(&pHead);
    
    if(iComplete == FINISHED){
      break;
    }
  }*/
  
  FreeList(&pHead);
}

int HandleMenu(FLIGHT_DEPARTURE **ppHead){
  int iComplete = CONTINUE;
  int iCh = getchar();
  //since getchar leaves \n in stdin, i flush stdin until \n or EOF
  int iFlush = 0;
  while((iFlush = getchar()) != '\n' && iFlush != EOF){}
  
  switch(iCh){
  
    case '1':
      printf("Enter flight id [format: XX-XX]:\n");
      char aszFlightId[6] = {0}; //+1 for zero terminator, +1 for newline
      if(ReadInput(aszFlightId, sizeof(aszFlightId)) != 0){
        break; //only break becaues printf error is in ReadInput
      }
      
      printf("Enter a destination flight %s:\n", aszFlightId);
      char aszDestination[BUFFER_SIZE] = {0};
      if(ReadInput(aszDestination, sizeof(aszDestination)) != 0){
        break;
      }
      
      printf("How many seats are available for this flight?\n");
      char aszSeats[4] = {0}; //max seats is 999, with \0 and \n read
      if(ReadInput(aszSeats, sizeof(aszSeats)) != 0){
        break;
      }
      int iSeats = atoi(aszSeats);
      if(iSeats == 0){
        printf("Invalid seat input, must be a number\n");
        break;
      }
      
      printf("What is the time for the departure? [format: HHMM]\n");
      char aszTime[5] = {0};
      if(ReadInput(aszTime, sizeof(aszTime)) != 0){
        break;
      }
      int iTime = atoi(aszTime);
      if(iTime == 0){
        printf("Invalid time input, must be a number (ex 1430)\n");
        break;
      }
      
      if(AddFlight(ppHead, aszFlightId, aszDestination, iSeats, iTime) != 0){
        printf("Failed to add flight departure to the list..\n");
      }
      PrintList(ppHead);

      break;
  
    case '2':
      //adding a passenger to a flight by its flightId
      printf("Enter the FLIGHT ID of the flight you want to ass this passenger on:\n");
      char aszFlight[6] = {0};
      PrintList(ppHead);
      if(ReadInput(aszFlight, 6) != 0){
        break;
      }
      
      FLIGHT_DEPARTURE *pFlight = GetFlightById(ppHead, aszFlight);
      if(pFlight == NULL){
        printf("You need to enter a valid flight id\n");
        break;
      }
      
      printf("What is the name of your passenger?\n");
      char aszName[BUFFER_SIZE] = {0};
      if(ReadInput(aszName, BUFFER_SIZE - 1) != 0){
        break;
      }
      
      printf("What is the age of your passenger?\n");
      char aszAge[4] = {0}; //people over 100 can fly here (not over 999 though)
      if(ReadInput(aszAge, 4) != 0){
        break;
      }
      int iAge = atoi(aszAge);
      if(iAge == 0){ //people that are 0 years old cannot fly apparantly
        printf("Enter a valid age! only numbers\n");
        break;
      }
      
      int iChosenSeat = 0;
      printf("Enter the seat you want, up to %d:\n", pFlight->iSeats);
      char aszSeat[4] = {0};
      if(ReadInput(aszSeat, 4) != 0){
        break;
      }
      int iSeat = atoi(aszSeat);

      /*while(1){ FIXME
        char aszSeat[4] = {0};
        if(ReadInput(aszSeat, 4) == 0){
          //if atoi fails, it retuns 0. 0 Is a valid seat, so i dont have to check the atoi value, because if the user types "sdfghj" the seat chosen will be 0 :)
          iChosenSeat = atoi(aszSeat);
          
          //now we have the seat, then loop through all passengers and see if the seat is taken
          PASSENGER *pPassenger = *(pFlight->ppPassengerHead);
          int iTaken = 0;
          while(pPassenger != NULL){
            if(iChosenSeat == pPassenger->iSeat){
              iTaken = 1;
              break;
            }
            pPassenger = pPassenger->pNext;
          }
          
          if(iTaken == 0){
            //if its not taken then we have found an available seat
            printf("Seat was available! Your spot is %d\n", iChosenSeat);
            break;
          } else {
            printf("That seat was already taken... Enter a new seat:\n");
            continue;
          }
        }
      }*/
      
      if(AddPassenger(pFlight, aszName, iAge, iChosenSeat) != 0){
        printf("Coulndt add passenger to this flight...\n");
      }
      PrintPassengers(pFlight);
      
      break;
      
    case '3':
      printf("Enter the index of which flight you want to see (first is 0):\n");
      char aszFlightIndex[4] = {0};
      if(ReadInput(aszFlightIndex, 4) != 0){
        break;
      }
      int iIndex = atoi(aszFlightIndex);
      
      RetrieveFlightPrintInfo(ppHead, iIndex);
      
      break;
      
    case '4':
      printf("Enter the destination you want to see flights for: \n");
      char aszDestinationMatch[BUFFER_SIZE] = {0};
      if(ReadInput(aszDestinationMatch, BUFFER_SIZE) != 0){
        break;
      }
      
      int iFlightIndex = MatchDepartureReturnIndex(ppHead, aszDestinationMatch);
      if(iFlightIndex >= 0){
        RetrieveFlightPrintInfo(ppHead, iFlightIndex);
      } else {
        printf("Could not find a flight to your destination\n");
      }
      break;
      
    case '5':
      break;
      
    case '6':
      break;
      
    case '7':
      break;
      
    case '8':
      break;
      
    case '9':
      iComplete = FINISHED;
      break;
      
    default:
      printf("You must type in a valid number...\n");
      break;
  }
  
  return iComplete;
}

int ReadInput(char *pszDestination, int iSize){
  if(fgets(pszDestination, iSize, stdin)){

    if(strchr(pszDestination, '\n') == NULL){
      //fgets did not read everything, so there are more chars left in stdin
      //i then have to flush the rest and manually zero terminate the last char
      pszDestination[iSize - 1] = '\0';
      int iFlush = 0;
      while((iFlush = getchar()) != '\n' && iFlush != EOF);
    } else {
      //if there is a \n, zero terminate it instead
      pszDestination[strcspn(pszDestination, "\n")] = '\0';
    }
    
    return 0;
  } else {
    printf("Could not read input\n");
    return 1;
  }
}

void PrintMenu(){
  printf("Welcome to the menu:) \n");
  printf("1 - Add a flight to the list\n");
  printf("2 - Add a passenger to a flight-departure\n");
  printf("3 - Retrieve information about a flight-departure\n");
  printf("4 - Find flights that matches departure destination\n");
  printf("5 - Delete a flight (with all its passengers)\n");
  printf("6 - Change the seat of a passenger\n");
  printf("7 - Search for a passengers name in all flights\n");
  printf("8 - Search through list for any passengers in all flights that are booked on more than 1 flight\n");
  printf("9 - Quit\n");
  printf("Enter your choice: \n");
}

//made this function just for testing during the coding
void PrintList(FLIGHT_DEPARTURE **ppHead){
  int i = 0;
  printf("------LIST OF FLIGHTS------\n");
  FLIGHT_DEPARTURE *pTemp = *ppHead;
  while(pTemp != NULL){
    printf("%d - %s\n", i, pTemp->aszFlightId);
    pTemp = pTemp->pNext;
    i++;
  }
}

void PrintPassengers(FLIGHT_DEPARTURE *pFlight){
  int i = 0;
  printf("------PASSENGERS ON FLIGHT %s------\n", pFlight->aszFlightId);
  PASSENGER *pTemp = *(pFlight->ppPassengerHead);
  while(pTemp != NULL){
    printf("%d - %s\n", i, pTemp->aszName);
    pTemp = pTemp->pNext;
    i++;
  }
}


