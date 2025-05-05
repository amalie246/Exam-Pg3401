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
  int iFinished; //mby delete
  int count[BYTE_RANGE];
  unsigned char buffer[BUFFER_SIZE];
  pthread_mutex_t mutex;
  sem_t semStart;
  sem_t semFinish;
  int bytes_in_buffer;
  void *memory_buffer;
} THREAD_DATA;

int totalBytesRead;

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
    printf("Bytes in buffer before reading: %d\n");
    //reading chars into buffer, size of char, 
    int read_bytes = fread(td->buffer + td->bytes_in_buffer, 1, 
    BUFFER_SIZE - td->bytes_in_buffer, fp);
    
    //bytes_in_buffer increases
    td->bytes_in_buffer += read_bytes;
    totalBytesRead +=read_bytes;
    printf("Bytes in buffer: %d\n", td->bytes_in_buffer);
    
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

  fclose(fp);
  pthread_exit(NULL);
}

void* thread_B(void* arg) {
  FILE *fh = fopen("task4_pg2265.hash", "w");
  if(fh == NULL){
    printf("Failed to open hash file\n");
  }

  FILE *ft = fopen("task4_pg2265.enc", "w");
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
    //The original code snippet from part 1
    for(int i = 0; i < td->bytes_in_buffer; i++){
      td->count[td->buffer[i]]++;
    }
    
    //check if bytes in buffer is 0 in a mutex protected section
    if(td->bytes_in_buffer == 0 || td->iFinished == 1){
      break;
    }

    //signal that thread A to start again
    td->bytes_in_buffer = 0;
    sem_post(&td->semStart);
    pthread_mutex_unlock(&td->mutex);
  }
  
  for(int i = 0; i < BYTE_RANGE; i++){
    printf("%d: %d\n", i, td->count[i]);
  }

  fclose(fh);
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

  printf("TOTAL BYTES READ: %d\n", totalBytesRead);
  return 0;
}







