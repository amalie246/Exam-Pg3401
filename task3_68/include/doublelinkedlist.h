#ifndef __DOUBLE_LINKED_LIST_H__
#define __DOUBLE_LINKED_LIST_H__

//includes for doublelinkedlist.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//macros
#define BUFFER_SIZE 124 //i doubt names and destinations are longer than this
#define OK 0
#define ERROR 1

//the structs i use for this task
typedef struct _PASSENGER{
  struct _PASSENGER *pNext;
  int iSeat;
  int iAge;
  char aszName[BUFFER_SIZE];
} PASSENGER;

typedef struct _FLIGHT_DEPARTURE{
  struct _FLIGHT_DEPARTURE *pNext;
  struct _FLIGHT_DEPARTURE *pPrev;
  PASSENGER **ppPassengerHead;
  char aszFlightId[6]; //ex: BA-42\0
  char aszDestination[BUFFER_SIZE];
  int iSeats;
  int iTime;
} FLIGHT_DEPARTURE;

//functions in doublelinkedlist.c
int AddFlight(FLIGHT_DEPARTURE **ppHead, char *pszFlightId, char *pszDestination, int iSeats, int iTime);
int AddPassenger(FLIGHT_DEPARTURE *pFlight, char *pszName, int iAge, int iSeat);
FLIGHT_DEPARTURE *GetFlightById(FLIGHT_DEPARTURE **ppHead, char *pszFlightId);
void RetrieveFlightPrintInfo(FLIGHT_DEPARTURE **ppHead, int iIndex);
int MatchDepartureReturnIndex(FLIGHT_DEPARTURE **ppHead, char *pszDestination);
int DeleteFlight(FLIGHT_DEPARTURE **ppHead, FLIGHT_DEPARTURE *pDelete);
int ChangePassengerSeat(FLIGHT_DEPARTURE *pFLight, char *pszName, int iSeat);
int SearchPassengerName(FLIGHT_DEPARTURE **ppHead, char *pszName);
void PrintPassengersOnMultipleFlights(FLIGHT_DEPARTURE **ppHead);
void FreeList(FLIGHT_DEPARTURE **ppHead);

#endif //__DOUBLE_LINKED_LIST_H__
