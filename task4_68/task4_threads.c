#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFER_SIZE 4096
#define NUM_THREADS 2
#define BYTE_RANGE 256

//struct to avoid global variables
typedef struct _THREAD_DATA{
  int count[BYTE_RANGE];
  unsigned char buffer[BUFFER_SIZE];
  pthread_mutex_t mutex;
  sem_t semStart;
  sem_t semFinish;
  int bytes_in_buffer;
  void *memory_buffer;
} THREAD_DATA;
/*
int count[BYTE_RANGE];
unsigned char buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
pthread_cond_t cond_full, cond_empty;
int bytes_in_buffer = 0;*/

void* thread_A(void* arg) {
printf("thread A started\n");
  FILE* fp = fopen("task4_pg2265.txt", "rb");
  if (!fp) {
     perror("Failed to open file");
     exit(EXIT_FAILURE);
  }
  printf("Opened file by thread A\n");
  THREAD_DATA *td = (THREAD_DATA*)arg;
  
  while(1){
    pthread_mutex_lock(&td->mutex);
    printf("Thread A: Locked mutex\n");
    
    while(td->bytes_in_buffer == BUFFER_SIZE){
      pthread_mutex_unlock(&td->mutex);
      printf("Thread A: unlocked mutex in while 2\n");
      sem_wait(&td->semStart);
      printf("Thread A: sent wait signal on semStart\n");
      pthread_mutex_lock(&td->mutex);
      printf("Thread A: locked mutex again in while 2\n");
    }
    
    int read_bytes = fread(td->buffer + td->bytes_in_buffer, 1, 
    BUFFER_SIZE - td->bytes_in_buffer, fp);
    
    if(read_bytes < BUFFER_SIZE - td->bytes_in_buffer){
      pthread_mutex_unlock(&td->mutex);
      printf("Thread A: unlocked mutex in if\n");
      break;
    }
    
    sem_post(&td->semFinish);
    printf("Thread A: sem post on FINISH\n");
    pthread_mutex_unlock(&td->mutex);
    printf("Thread A: unlocked mutex in the end\n");
  }

  /*while (1) {
     pthread_mutex_lock(&td->mutex);
     while (bytes_in_buffer == BUFFER_SIZE)  
        pthread_cond_wait(&cond_empty, &td->mutex);

     int read_bytes = fread(buffer + bytes_in_buffer, 1, BUFFER_SIZE - bytes_in_buffer, fp);
     bytes_in_buffer += read_bytes;

     if (read_bytes < BUFFER_SIZE - bytes_in_buffer) {
        pthread_mutex_unlock(&mutex);
        break;
     }
     //pthread_cond_signal(&cond_full);
     pthread_mutex_unlock(&mutex);
  }*/
  fclose(fp);
  pthread_exit(NULL);
}

void* thread_B(void* arg) {
printf("thread B started\n");
  THREAD_DATA *td = (THREAD_DATA*)arg;
  memset(td->count, 0, sizeof(td->count));
  printf("thread B memset count\n");
  
  while(1){
    pthread_mutex_lock(&td->mutex);
    printf("Thread B: locked mutex\n");
    
    while(td->bytes_in_buffer == 0){
      pthread_mutex_unlock(&td->mutex);
      printf("Thread B: unlocked mutex in while 2\n");
      sem_wait(&td->semFinish);
      printf("Thread B: sent wait signal on Finish\n");
      pthread_mutex_lock(&td->mutex);
      printf("Thread B: locked mutex again in while 2\n");
    }
    
    for(int i = 0; i < td->bytes_in_buffer; i++)
      td->count[td->buffer[i]]++;
    
    td->bytes_in_buffer = 0;
    sem_post(&td->semFinish);
    printf("Thread B: sent signal to Finish\n");
    pthread_mutex_unlock(&td->mutex);
    
    if(td->bytes_in_buffer == 0){
      break;
    }
    
  }
  
  for(int i = 0; i < BYTE_RANGE; i++){
    printf("%d: %d\n", i, td->count[i]);
  }
  pthread_exit(NULL);
  /*memset(count, 0, sizeof(count));

  while (1) {
     pthread_mutex_lock(&mutex);
     while (bytes_in_buffer == 0)
        pthread_cond_wait(&cond_full, &mutex);

     for (int i = 0; i < bytes_in_buffer; i++)
        count[buffer[i]]++;

     bytes_in_buffer = 0;
     //pthread_cond_signal(&cond_empty);
     pthread_mutex_unlock(&mutex);

     if (bytes_in_buffer == 0)
        break;
  }
  for (int i = 0; i < BYTE_RANGE; i++)
     printf("%d: %d\n", i, count[i]);
  pthread_exit(NULL);*/
}

int main(void) {
  pthread_t threadA, threadB;
   
  //created a struct for both threads so they can share memory buffer, semaphores etc
  THREAD_DATA *td = malloc(sizeof(THREAD_DATA));   
  if(td == NULL){
    printf("Malloc failed on struct\n");
    return 1;
  }
  td->bytes_in_buffer = 0;
  td->memory_buffer = malloc(BUFFER_SIZE);
  if(td->memory_buffer == NULL){
    printf("Malloc failed on struct member memory_buffer\n");
    return 1;
  }
  //void* memory_buffer = malloc(BUFFER_SIZE);
   
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

  return 0;
}










