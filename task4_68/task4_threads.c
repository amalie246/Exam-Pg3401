#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "include/task4_threads.h"

#define BUFFER_SIZE 4096
#define NUM_THREADS 2
#define BYTE_RANGE 256

//struct to avoid global variables
typedef struct _THREAD_DATA{
  char aszFileName[124]; //doubt filename is larger than this
  FILE *fInputFile;
  int iFinished; //set a flag for when thread A has read the entire file
  int count[BYTE_RANGE];
  unsigned char buffer[BUFFER_SIZE];
  pthread_mutex_t mutex;
  sem_t semStart;
  sem_t semFinish;
  int bytes_in_buffer;
  void *memory_buffer;
} THREAD_DATA;

//thread A calls this function to hash the file after the entire file is read
void Hash(THREAD_DATA *td){
  //fh stands for File Hash
  FILE *fh = fopen(td->aszFileName, "w");
  if(fh == NULL){
    printf("Failed to open hash file\n");
  }

  //rewinding the file
  rewind(td->fInputFile);
  
  //same concept as in task2_68/main.c
  int iHash = 0;
  char byHash[4] = {0};
  Task2_SimpleDjb2Hash(td->fInputFile, &iHash);
  memcpy(&byHash, &iHash, 4);
  fwrite(byHash, sizeof(byHash), 1, fh);

  fclose(fh);
}

//thread B calls this to encrypt BUFFER_SIZE amounts of data from the file
void TEA(FILE *ft, THREAD_DATA *td){
  //in the documentation for tea.c, it says it takes in 64-bit blocks
  //therefore, I assume I can read the buffer in 64-bit chunks
  //and pad the last chunk IF it is not the full 64 bit
  int iTotalBytes = 0, iBufferIndex = 0;
  while(iTotalBytes <= td->bytes_in_buffer){
    //read 8 bytes at a time (8 bytes is 64 bits)
    char cBuf[8] = {0}; //char is 1 byte
    int iBytesRead = 0;
    

    //only memcpy the full 64 bit if 
    if(iBufferIndex + 8 <= sizeof(td->buffer)){
      memcpy(cBuf, td->buffer + iBufferIndex, 8);
      iBufferIndex += 8;
      iBytesRead = 8;
    } else {
      int iBytesLeft = sizeof(td->buffer) - iBufferIndex;
      memcpy(cBuf, td->buffer + iBufferIndex, iBytesLeft);
      iBytesRead = iBytesLeft;
    }
    
    iTotalBytes += iBytesRead;
    //how many bytes needs padding
    int iNeedsPadding = 8 - iBytesRead;
    
    //found out what pkcs5 padding is here, under method 1:
    //https://www.di-mgt.com.au/cryptopad.html
    if(iNeedsPadding != 0){
      //needs to pad the last x bytes in cBuf
      for(int i = iBytesRead; i < 8; i++){
        //since iNeedsPadding can only be 1-7 here, it should be safe to place it a char
        cBuf[i] = iNeedsPadding; // fill rest with ex. 1 if 7 bytes are read
      }
    }
    
    //convert it into unsigned int now that cBuf is guaranteed full
    unsigned int uiV[2] = {0};
    memcpy(&uiV[0], cBuf, 4);
    memcpy(&uiV[1], cBuf + 4, 4);
    
    //i understood that tea needs 128 bits keys, key[0] - key[3], however i do not
    //understand how to generate these, so I am just writing random stuff in here
    unsigned int uiKey[4] = {0};
    uiKey[0] = 10;
    uiKey[1] = 400;
    uiKey[2] = 75;
    uiKey[3] = 9999;
    
    //i believe this is what is supposed to be written into the encrypted file
    unsigned int uiW[2] = {0};
    encipher(uiV, uiW, uiKey);
    
    //now write it to the file    
    fwrite(uiW, sizeof(uiW), 1, ft);
    if(iBytesRead < 8){
      break;
    }
  }
}

void* thread_A(void* arg) {
  //cast void pointer to THREAD_DATA struct
  THREAD_DATA *td = (THREAD_DATA*)arg;
  //open file (original code)
  FILE* fp = fopen(td->aszFileName, "rb");
  if (!fp) {
     perror("Failed to open file");
     exit(EXIT_FAILURE);
  }

  //setting the structs file to fp file descriptor
  td->fInputFile = fp;
  
  while(1){
    //locking the mutex
    pthread_mutex_lock(&td->mutex); 
    
    //this is while Thread B is working
    while(td->bytes_in_buffer == BUFFER_SIZE){
      //must manually unlock the mutex, sem_wait does not do it the same way
      //that pthread_cond_wait() does
      pthread_mutex_unlock(&td->mutex);
      //wait here until thread B
      sem_wait(&td->semStart);
      //lock mutex again
      pthread_mutex_lock(&td->mutex);
    }
    
    //reading chars into buffer, size of char, 
    int read_bytes = fread(td->buffer + td->bytes_in_buffer, 1, 
    BUFFER_SIZE - td->bytes_in_buffer, fp);
    
    //bytes_in_buffer increases
    td->bytes_in_buffer += read_bytes;
    
    if(read_bytes < BUFFER_SIZE - td->bytes_in_buffer){
      //finished reading the entire file
      td->iFinished = 1;
      pthread_mutex_unlock(&td->mutex);
      sem_post(&td->semFinish);
      break;
    }
    //send signal to thread B that thread A is finished, and unlock mutex
    sem_post(&td->semFinish);
    pthread_mutex_unlock(&td->mutex);
  }

  //hashing the function after thread A is finished, but before it closes the file
  Hash(td);

  fclose(fp);
  pthread_exit(NULL);
}

void* thread_B(void* arg) {
  //open the encrypted file
  FILE *ft = fopen("task4_pg2265.enc", "wb");
  if(ft == NULL){
    printf("Failed to open TEA file\n");
  }
  //cast void pointer to THREAD DATA struct
  THREAD_DATA *td = (THREAD_DATA*)arg;
  memset(td->count, 0, sizeof(td->count));
  
  while(1){
    pthread_mutex_lock(&td->mutex);
    
    while(td->bytes_in_buffer == 0){
      //wait for thread A to signal that it is finished so B can work
      pthread_mutex_unlock(&td->mutex);
      sem_wait(&td->semFinish);
      pthread_mutex_lock(&td->mutex);
    }

    //saved the value here instead of fetching the values in the if-clause below,
    //because it might be changed by thread A outside the mutex protection
    int iFinished = td->iFinished;
    int iBytesInBuffer = td->bytes_in_buffer;

    //The original code snippet from part 1
    /*for(int i = 0; i < td->bytes_in_buffer; i++){
      td->count[td->buffer[i]]++;
    }*/

    //Thread B can call TEA function here and send in the buffer
    TEA(ft, td);   

    //signal that thread A to start again
    td->bytes_in_buffer = 0;
    sem_post(&td->semStart);
    pthread_mutex_unlock(&td->mutex);

    //use values saved inside mutex lock to check if thread B should exit
    if(iFinished != 0 || iBytesInBuffer == 0){
      break;
    }
  }
  
  //Original code snippet from part 1
  /*for(int i = 0; i < BYTE_RANGE; i++){
    printf("%d: %d\n", i, td->count[i]);
  }*/

  fclose(ft);
  pthread_exit(NULL);
}

int main(int iArgC, char *apszArgV[]) {
  pthread_t threadA, threadB;

  //created a struct for both threads so they can share memory buffer, semaphores etc
  THREAD_DATA *td = malloc(sizeof(THREAD_DATA));   
  if(td == NULL){
    printf("Malloc failed on struct\n");
    return 1;
  }
  td->bytes_in_buffer = 0;
  td->iFinished = 0; //should be 1 when its completely done reading
  td->memory_buffer = malloc(BUFFER_SIZE);
  if(td->memory_buffer == NULL){
    printf("Malloc failed on struct member memory_buffer\n");
    return 1;
  }
  
  if(iArgC == 2){
    memcpy(td->aszFileName, apszArgV[1], 124);
  } else {
    printf("Must specify file name!!!\n");
    return 1;
  }
   
  //initialize semaphores
  if(sem_init(&td->semStart, 0, 0) != 0){
    printf("Failed to create semaphore 1\n");
    return 1;
  }
  if(sem_init(&td->semFinish, 0, 0) != 0){
    printf("Failed to create sempahore 2\n");
    return 1;
  }
   
  //initialize mutex
  if(pthread_mutex_init(&td->mutex, NULL)){
    printf("Failed to create mutex\n");
    return 1;
  }

  if (pthread_create(&threadA, NULL, thread_A, (void*)td) != 0) {
     perror("Could not create thread A");
     exit(1);
  }

  if (pthread_create(&threadB, NULL, thread_B, (void*)td) != 0) {
     perror("Could not create thread B");
     exit(1);
  }

  if (pthread_join(threadA, NULL) != 0) {
     perror("Could not join thread A");
     exit(1);
  }
  if (pthread_join(threadB, NULL) != 0) {
     perror("Could not join thread B");
     exit(1);
  }
  
  pthread_mutex_destroy(&td->mutex);
  free(td->memory_buffer);
  free(td);
  td = NULL;

  return 0;
}







