#ifndef __MAIN_H__
#define __MAIN_H__

#define OK 0
#define ERROR 1

struct TASK2_FILE_METADATA{
  char szFileName[32];
  int iFileSize;
  char byHash[4];
  int iSumOfChars;
  char aAlphaCount[26];
}

#endif //__MAIN_H__
