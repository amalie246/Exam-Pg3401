#include "include/doublelinkedlist.h"

FLIGHT_DEPARTURE *CreateFlight(char *pszFlightId, char *pszDestination, int iSeats, int iTime){
  FLIGHT_DEPARTURE *pFd = malloc(sizeof(FLIGHT_DEPARTURE));
  if(pFd == NULL){
    printf("Malloc failed to create FLIGHT_DEPARTURE\n");
    return NULL;
  }
  
  pFd->iSeats = iSeats;
  pFd->iTime = iTime;
  
  memcpy(pFd->aszFlightId, pszFlightId, strlen(pszFlightId));
  memcpy(pFd->aszDestination, pszDestination, strlen(pszDestination));
  
  return pFd;
}

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
  
  //allocate memory for ppPassengerHead so you dont access NULL value
  if(pFlight->ppPassengerHead == NULL){
    pFlight->ppPassengerHead = malloc(sizeof(PASSENGER*));
    if(pFlight->ppPassengerHead == NULL){
      printf("Malloc failed on first passenger\n");
      return iRc;
    }
    *(pFlight->ppPassengerHead) = NULL; //now it points to NULL
  }

  //if there are no passenger or pThis should be before head
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
  }
  
  return iRc;
}

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
    if(iCount == iIndex){
      printf("\nFlight found:\n");
      printf("ID:         \t%s\n", pTemp->aszFlightId);
      printf("Destination:\t%s\n", pTemp->aszDestination);
      printf("Seats:      \t%d\n", pTemp->iSeats);
      printf("Time:       \t%d\n", pTemp->iTime);
      printf("Passengers: \t");
      
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
  
  if(*ppHead == pDelete){
    (*ppHead = pDelete->pNext);
    iRc = OK;
  } else {
    FLIGHT_DEPARTURE *pTemp = *ppHead;
    while(pTemp != NULL){
      if(pTemp == pDelete){
        if(pTemp->pPrev != NULL){
          pTemp->pPrev->pNext = pTemp->pNext;
        }
        if(pTemp->pNext != NULL){
          pTemp->pNext->pPrev = pTemp->pPrev;
        }
        
        //free the passengers associated to the flight, and then free flight
        PASSENGER *pPassenger = *(pTemp->ppPassengerHead);
        while(pPassenger != NULL){
          PASSENGER *pPassengerNext = pPassenger->pNext;
          free(pPassenger);
          pPassenger = pPassengerNext; 
        }
        pPassenger = NULL;
        free(pTemp);
        pTemp = NULL;
      }
      pTemp = pTemp->pNext;
    }
  }
  
  return iRc;
}

void FreeList(FLIGHT_DEPARTURE **ppHead){
  FLIGHT_DEPARTURE *pTemp = *ppHead;
  
  while(pTemp != NULL){
    FLIGHT_DEPARTURE *pTempNext = pTemp->pNext;
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
  
}




