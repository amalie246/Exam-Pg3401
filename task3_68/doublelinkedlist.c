#include "include/doublelinkedlist.h"

//returns pointer to a flight struct
FLIGHT_DEPARTURE *CreateFlight(char *pszFlightId, char *pszDestination, int iSeats, int iTime){
  FLIGHT_DEPARTURE *pFd = malloc(sizeof(FLIGHT_DEPARTURE));
  if(pFd == NULL){
    printf("Malloc failed to create FLIGHT_DEPARTURE\n");
    return NULL;
  }
  
  pFd->iSeats = iSeats;
  pFd->iTime = iTime;
  
  memcpy(pFd->aszFlightId, pszFlightId, 6);
  memcpy(pFd->aszDestination, pszDestination, BUFFER_SIZE);
  
  return pFd;
}

//returns pointer to passenger stuct
PASSENGER *CreatePassenger(char *pszName, int iAge, int iSeat){
  PASSENGER *pP = malloc(sizeof(PASSENGER));
  if(pP == NULL){
    printf("Malloc failed to create PASSENGER\n");
    return NULL;
  }
  
  pP->iAge = iAge;
  pP->iSeat = iSeat;
  
  memcpy(pP->aszName, pszName, strlen(pszName));
  
  return pP;
}

int AddFlight(FLIGHT_DEPARTURE **ppHead, char *pszFlightId, char *pszDestination, int iSeats, int iTime){
  int iRc = ERROR;
  FLIGHT_DEPARTURE *pThis = CreateFlight(pszFlightId, pszDestination, iSeats, iTime);
  if(pThis == NULL){
    printf("CreateFlight function failed!\n");
    return iRc;
  }
  
  //first, checking if this is the first flight in the list
  if(*ppHead == NULL){
    *ppHead = pThis;
    iRc = OK;
  } else {
    //if not, then i find the last element in the list and place this flight after
    FLIGHT_DEPARTURE *pTemp = *ppHead;
    while(pTemp->pNext != NULL){
      pTemp = pTemp->pNext;
    }
    
    //now pTemp is at second to last position, so we place pThis after it
    pTemp->pNext = pThis;
    pThis->pPrev = pTemp;
  }
  
  return iRc;
}

int AddPassenger(FLIGHT_DEPARTURE *pFlight, char *pszName, int iAge, int iSeat){
  int iRc = ERROR;
  
  if(pFlight == NULL){
    return iRc;
  }
  
  PASSENGER *pThis = CreatePassenger(pszName, iAge, iSeat);
  if(pThis == NULL){
    printf("CreatePassenger failed\n");
  }
  
  //if trying to access *(pFLight->ppPassengerHead) it crashes if it is NULL
  //it needs memory first, then it can point to a NULL value instead
  if(pFlight->ppPassengerHead == NULL){
    pFlight->ppPassengerHead = malloc(sizeof(PASSENGER*));
    if(pFlight->ppPassengerHead == NULL){
      printf("Malloc failed on first passenger\n");
      return iRc;
    }
    *(pFlight->ppPassengerHead) = NULL; //now it POINTS to NULL
  }

  //if there are no passengers or pThis should be before head
  if(*(pFlight->ppPassengerHead) == NULL ||
    (*(pFlight->ppPassengerHead))->iSeat > iSeat){
    
    pThis->pNext = *(pFlight->ppPassengerHead);
    *(pFlight->ppPassengerHead) = pThis;
    iRc = OK;
    
  } else {
    PASSENGER *pTemp = *(pFlight->ppPassengerHead);
    //traverse until end of list or pTemps nexts seat is larger than pThis seat
    while(pTemp->pNext != NULL && pTemp->pNext->iSeat < iSeat){
      pTemp = pTemp->pNext;
    }
    
    //place pThis in between temp and its next
    pThis->pNext = pTemp->pNext;
    pTemp->pNext = pThis;
    iRc = OK;
  }
  
  return iRc;
}

//returns pointer to flight struct
FLIGHT_DEPARTURE *GetFlightById(FLIGHT_DEPARTURE **ppHead, char *pszFlightId){
  //traversing through the list until it finds the flight with corrrect flight id
  FLIGHT_DEPARTURE *pFlight = *ppHead;
  while(pFlight != NULL){
    if(strcmp(pFlight->aszFlightId, pszFlightId) == 0){
      return pFlight;
    }
    pFlight = pFlight->pNext;
  }
  return NULL;
}

void RetrieveFlightPrintInfo(FLIGHT_DEPARTURE **ppHead, int iIndex){
  FLIGHT_DEPARTURE *pTemp = *ppHead;
  
  int iCount = 0;
  while(pTemp != NULL && iCount <= iIndex){
    //traverse until the correct position
    if(iCount == iIndex){
      printf("\nFlight found:\n");
      printf("ID:         \t%s\n", pTemp->aszFlightId);
      printf("Destination:\t%s\n", pTemp->aszDestination);
      printf("Seats:      \t%d\n", pTemp->iSeats);
      printf("Time:       \t%d\n", pTemp->iTime);
      printf("Passengers: \t");
      
      //must check first if there are any passengers, trying to access passengers when there is none will crash
      if(pTemp->ppPassengerHead == NULL){
        printf("[No passengers]\n");
      } else {
        PASSENGER *pPassenger = *(pTemp->ppPassengerHead);
        while(pPassenger != NULL){
          printf("\n - %s, %dyears, seat: %d", pPassenger->aszName, pPassenger->iAge, pPassenger->iSeat);
          pPassenger = pPassenger->pNext;
        }
        printf("\n");
      }
      
      break;
    }
    pTemp = pTemp->pNext;
    iCount++;
  }
}

//returns position of a flight that matches pszDestination
int MatchDepartureReturnIndex(FLIGHT_DEPARTURE **ppHead, char *pszDestination){
  int iIndex = -1, iCount = 0;
  if(*ppHead == NULL){
    return iIndex;
  }
  
  FLIGHT_DEPARTURE *pTemp = *ppHead;
  while(pTemp != NULL){
    if(strcmp(pTemp->aszDestination, pszDestination) == 0){
      return iCount;
    }
    pTemp = pTemp->pNext;
    iCount++;
  }
  
  return iIndex;
}

int DeleteFlight(FLIGHT_DEPARTURE **ppHead, FLIGHT_DEPARTURE *pDelete){
  int iRc = ERROR;

  if(*ppHead == NULL){
    return iRc;
  }
  //temporary save this so it can be freed if either if or else clause happens
  FLIGHT_DEPARTURE *pTemp = *ppHead; 

  if(*ppHead == pDelete){ //deleting head
    (*ppHead = pDelete->pNext);
    iRc = OK;
  } else {
    //traversing until we have found the correct flight
    while(pTemp != NULL){
      if(pTemp == pDelete){
        //fixing the previous flight is there is any
        if(pTemp->pPrev != NULL){
          pTemp->pPrev->pNext = pTemp->pNext;
        }
        
        //fixing the next flight if there is any
        if(pTemp->pNext != NULL){
          pTemp->pNext->pPrev = pTemp;
        }
        iRc = OK;
        break;
      }
      pTemp = pTemp->pNext;
    }
  }
  //free passengers if they exist
  if(pTemp->ppPassengerHead != NULL){
    PASSENGER *pPassenger = *(pTemp->ppPassengerHead);
    while(pPassenger != NULL){
      PASSENGER *pPassengerNext = pPassenger->pNext;
      free(pPassenger);
      pPassenger = pPassengerNext;
    }
    free(pTemp->ppPassengerHead);
  }
  //free the flight
  free(pTemp);
  pTemp = NULL;
  
  return iRc;
}

//helper function for ChangePassengerSeat
int SortPassengerList(FLIGHT_DEPARTURE *pFlight, PASSENGER *pPassenger){
  //first delete the passenger in its original position from the list
  PASSENGER *pTemp = *(pFlight->ppPassengerHead);
  PASSENGER *pPrev = NULL;
  if(pTemp == pPassenger){
    //it is head, remove from list and point to second passenger instead
    *(pFlight->ppPassengerHead) = pTemp->pNext; 
  } else {
    while(pTemp != NULL){
      if(pTemp == pPassenger){
      //we have found its position, now delete it
        pPrev->pNext = pPassenger->pNext;
      }
      pPrev = pTemp;
      pTemp = pTemp->pNext;
    }
  }
  //then insert it into the right place again
  if(*(pFlight->ppPassengerHead) == NULL || 
  (*(pFlight->ppPassengerHead))->iSeat > pPassenger->iSeat){
    pPassenger->pNext = *(pFlight->ppPassengerHead);
    *(pFlight->ppPassengerHead) = pPassenger;
  } else {
    pTemp = *(pFlight->ppPassengerHead);
    while(pTemp->pNext != NULL && pTemp->pNext->iSeat < pPassenger->iSeat){
      pTemp = pTemp->pNext;
    }
    pPassenger->pNext = pTemp->pNext;
    pTemp->pNext = pPassenger;    
  }

}

int ChangePassengerSeat(FLIGHT_DEPARTURE *pFlight, char *pszName, int iSeat){
  int iRc = ERROR;
  if(pFlight == NULL){
    return iRc;
  }
  
  if(pFlight->ppPassengerHead == NULL){
    return iRc;
  } else {
    PASSENGER *pPassenger = *(pFlight->ppPassengerHead);
    //loop until the correct passenger is found
    while(pPassenger != NULL){
      if(strcmp(pPassenger->aszName, pszName) == 0){
        //the correct passenger is found, change his seat
        pPassenger->iSeat = iSeat;
        //now, the passenger probably needs to change position in the list
        SortPassengerList(pFlight, pPassenger);
        iRc = OK;
      }
      pPassenger = pPassenger->pNext;
    }
  }
  
  return iRc;
}

int SearchPassengerName(FLIGHT_DEPARTURE **ppHead, char *pszName){
  int iRc = ERROR;
  if(*ppHead == NULL){
    return iRc; //no flights to search
  }
  
  int iCount = 0;
  //loop through all flights and their passengers and match the name
  FLIGHT_DEPARTURE *pFlight = *ppHead;
  while(pFlight != NULL){
    
    if(pFlight->ppPassengerHead != NULL){
      PASSENGER *pPassenger = *(pFlight->ppPassengerHead);
      while(pPassenger != NULL){
        if(strcmp(pPassenger->aszName, pszName) == 0){
          //found passenger at index iCount, then print the info
          printf("-PASSENGER MATCH FOUND-\n");
          printf("Flight %d - %s\n", iCount, pFlight->aszFlightId);
          printf("Destination: %s\n", pFlight->aszDestination);
          printf("Time: %d\n", pFlight->iTime);
          printf("Seats: %d\n", pFlight->iSeats);
          printf("-----------------------\n");
          iRc = OK;
          break;
        }
        
        pPassenger = pPassenger->pNext;
      }
    }
    iCount++;
    pFlight = pFlight->pNext;
  }
  
  return iRc;
}

//helper funtcion for PrintPassengersOnMultipleFlights
int CountPassengers(FLIGHT_DEPARTURE *pFlight){
  int iTotal = 0;
  
  if(pFlight->ppPassengerHead != NULL){
    PASSENGER *pThis = *(pFlight->ppPassengerHead);
    
    while(pThis != NULL){
      iTotal++;
      pThis = pThis->pNext;
    }
  }
  
  return iTotal;
}
//also a helper function for PrintPassengersOnMultipleFlights
int CheckDuplicateEntries(char **ppNames, char *pszName){
  for(int i = 0; ppNames[i] != NULL; i++){
    if(strcmp(ppNames[i], pszName) == 0){
      return 1;
    }
  }
  
  //returns 0 if there isnt a duplicate entry, 1 if it is
  return 0;
}

void PrintPassengersOnMultipleFlights(FLIGHT_DEPARTURE **ppHead){
  //temporary dyamic list of strings to store unique names in passenger list
  char **ppNames = NULL;
  
  //find out how much memory to allocate
  int iTotalPassengers = 0;
  FLIGHT_DEPARTURE *pHead = *ppHead;
  while(pHead != NULL){
    iTotalPassengers += CountPassengers(pHead);
    pHead = pHead->pNext;
  }
  
  //malloc size for the entire array
  ppNames = malloc(sizeof(char*) * iTotalPassengers);
  if(ppNames == NULL){
    printf("Malloc failed on entire array\n");
  }
  
  //then allocate enough memory for each passengers name for the list
  int i = 0;
  for(i = 0; i < iTotalPassengers; i++){
    ppNames[i] = malloc(sizeof(char) * BUFFER_SIZE);
    if(ppNames[i] == NULL){
      printf("Malloc failed to allocate memory for a name\n");
      return;
    }
  }
  i = 0;
  //then copy unique names into the array, if the name is already in the array, print it
  FLIGHT_DEPARTURE *pFlight = *ppHead;
  while(pFlight != NULL){
    if(pFlight->ppPassengerHead != NULL){
      PASSENGER *pPassenger = *(pFlight->ppPassengerHead);
      while(pPassenger != NULL){
        if(CheckDuplicateEntries(ppNames, pPassenger->aszName) == 0){
          //name is not in the list, so copy the name into the list
          memcpy(ppNames[i], pPassenger->aszName, BUFFER_SIZE);
          i++;
        } else {
          //the passenger is already booked on 1 flight, so now it can be printed!
          printf("%s\n", pPassenger->aszName);
        }
        pPassenger = pPassenger->pNext;
      }
    }
    pFlight = pFlight->pNext; 
  }
}

//frees every flight with its passengers
void FreeList(FLIGHT_DEPARTURE **ppHead){
  FLIGHT_DEPARTURE *pTemp = *ppHead;
  
  while(pTemp != NULL){
    FLIGHT_DEPARTURE *pTempNext = pTemp->pNext;
    //only try to free passengers if they exist
    if(pTemp->ppPassengerHead != NULL){
      PASSENGER *pPassTemp = *(pTemp->ppPassengerHead);
      
      while(pPassTemp != NULL){
        PASSENGER *pPassTempNext = pPassTemp->pNext;
        free(pPassTemp);
        pPassTemp = pPassTempNext;
      }
      
      free(pTemp->ppPassengerHead);
    }
    
    free(pTemp);
    pTemp = pTempNext;
  }
  pTemp = NULL;
}




