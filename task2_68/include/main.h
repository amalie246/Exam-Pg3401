#ifndef __MAIN_H__
#define __MAIN_H__

#define OK 0
#define ERROR 1

#define FILENAME "pgexam25_test.txt"

#pragma pack(1)
struct TASK2_FILE_METADATA{
  char szFileName[32]; //name of input file, i assume
  int iFileSize; //size of input file
  char byHash[4];
  int iSumOfChars; 
  char aAlphaCount[26];
};

#pragma pack()

#endif //__MAIN_H__
