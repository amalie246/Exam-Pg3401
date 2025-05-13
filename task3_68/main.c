#include <stdio.h>
#include <string.h>
#include "include/doublelinkedlist.h"

#define CONTINUE 0
#define FINISHED 1

void PrintMenu();
int HandleMenu(FLIGHT_DEPARTURE **ppHead);
void PrintList(FLIGHT_DEPARTURE **ppHead);
int ReadInput(char *pszDestination, int iSize);
void PrintPassengers(FLIGHT_DEPARTURE *pFlight);

int main(int iArgC, char *apszArgV[]){
  int iComplete = CONTINUE;
  FLIGHT_DEPARTURE *pHead = {0};

  printf("\nWelcome to the menu:) \n");
  while(iComplete != FINISHED){
    PrintMenu();
    iComplete = HandleMenu(&pHead);
    
    if(iComplete == FINISHED){
      break;
    }
  }
  
  FreeList(&pHead);
}

int HandleMenu(FLIGHT_DEPARTURE **ppHead){
  int iComplete = CONTINUE, iRc = ERROR;
  int iCh = getchar();
  //since getchar leaves \n in stdin, i flush stdin until \n or EOF
  int iFlush = 0;
  while((iFlush = getchar()) != '\n' && iFlush != EOF){}
  
  switch(iCh){
  
    case '1':
      printf("Enter flight id [format: XX-XX]:\n");
      char aszFlightId[6] = {0}; //+1 for zero terminator
      if(ReadInput(aszFlightId, sizeof(aszFlightId)) != 0){
        break; //only break becaues printf error is in ReadInput
      }
      
      printf("Enter a destination flight %s:\n", aszFlightId);
      char aszDestination[BUFFER_SIZE] = {0};
      if(ReadInput(aszDestination, sizeof(aszDestination)) != 0){
        break;
      }
      
      printf("How many seats are available for this flight?\n");
      char aszSeats[4] = {0}; //max seats is 999, with \0
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
      PrintList(ppHead);
      printf("Enter the FLIGHT ID of the flight you want to ass this passenger on:\n");
      char aszFlight[6] = {0};
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
      
      if(AddPassenger(pFlight, aszName, iAge, iSeat) != 0){
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
      printf("Enter the flight ID to the flight you want to delete:\n");
      char aszDeleteFlightId[6] = {0};
      if(ReadInput(aszDeleteFlightId, 6) != 0){
        break;
      }
      FLIGHT_DEPARTURE *pDelete = GetFlightById(ppHead, aszDeleteFlightId);
      if(pDelete == NULL){
        printf("No such Flight Id in the list\n");
        break;
      }
      
      if(DeleteFlight(ppHead, pDelete) == ERROR){
        printf("Coudlnt delete your flight...\n");
      } else {
        printf("Deleted flight %s, the list is now: \n", aszDeleteFlightId);
        PrintList(ppHead);
      }

      break;
      
    case '6':
      printf("Which passenger do you want to change the seat for? Enter name: \n");
      char aszNameChangeSeat[BUFFER_SIZE] = {0};
      if(ReadInput(aszNameChangeSeat, BUFFER_SIZE) != 0){
        break;
      }
      
      printf("Which flight associated with this passenger? Enter id:\n");
      char aszSearchFlightId[6] = {0};
      if(ReadInput(aszSearchFlightId, 6) != 0){
        break;
      }
      
      printf("Which seat do you want to switch to?\n");
      char aszNewSeat[4] = {0};
      if(ReadInput(aszNewSeat, 4) != 0){
        break;
      }
      int iNewSeat = atoi(aszNewSeat);
      
      FLIGHT_DEPARTURE *pFlightChangeSeat = GetFlightById(ppHead, aszSearchFlightId);
      if(pFlightChangeSeat == NULL){
        printf("No flight associated with the ID you entered\n");
      } else {
        iRc = ChangePassengerSeat(pFlightChangeSeat, aszNameChangeSeat, iNewSeat);
        if(iRc == ERROR){
          printf("Could not change passenger seat, is the passenger name valid?\n");
        } else {
          printf("Changed passenger seat:\n");
          PrintPassengers(pFlightChangeSeat);
        }
      }
   
      break;
      
    case '7':
      printf("Enter the name you want to search for: \n");
      char aszSearchName[BUFFER_SIZE] = {0};
      if(ReadInput(aszSearchName, BUFFER_SIZE) != 0){
        break;
      }
      iRc = SearchPassengerName(ppHead, aszSearchName);
      if(iRc == ERROR){
        printf("No such passengers in the list\n");
      }
      break;
      
    case '8':
      printf("List of all passengers that are booked on more than one flight: \n");
      PrintPassengersOnMultipleFlights(ppHead);
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

//writes into a buffer, returns int for error handling
int ReadInput(char *pszDestination, int iSize){
  if(fgets(pszDestination, iSize, stdin)){

    if(strchr(pszDestination, '\n') == NULL){
      //fgets did not read everything if the \n wasnt read, so there are more chars
      //left in stdin
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
  printf("\n1 - Add a flight to the list\n");
  printf("2 - Add a passenger to a flight-departure\n");
  printf("3 - Retrieve information about a flight-departure\n");
  printf("4 - Find flights that matches departure destination\n");
  printf("5 - Delete a flight (with all its passengers)\n");
  printf("6 - Change the seat of a passenger\n");
  printf("7 - Search for a passengers name in all flights\n");
  printf("8 - Search through list for any passengers in all flights that are booked on more than 1 flight\n");
  printf("9 - Quit\n");
  printf("Enter your choice: \n\n");
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
    printf("%d - %s, age %d, seat %d\n", i, pTemp->aszName, pTemp->iAge, pTemp->iSeat);
    pTemp = pTemp->pNext;
    i++;
  }
}


