#ifndef SPIKETESTS_H
#define SPIKETESTS_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "itype.h"
#include "que.h"

/***********************************************


            Utility Methods


/***********************************************/

void Sleep(time_t seconds, long nanos)
{
  struct timespec tspec;
  tspec.tv_sec = seconds;
  tspec.tv_nsec = nanos;

  nanosleep(&tspec, NULL);
}

/***********************************************


             Non-Blocking Test


/***********************************************/

void* ProducerWriteNonBlocking(void* qptr)
{
  ThreadSpike::ThreadQueue& queue = *(ThreadSpike::ThreadQueue*)qptr;
  u8 writeBytes[THREADQUEUE_MAX_BYTES];
  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES; i++)
  {
    writeBytes[i] = i;
  }
  queue.Write(writeBytes, THREADQUEUE_MAX_BYTES);
  return NULL;
}

void* ConsumerReadNonBlocking(void* qptr)
{
  ThreadSpike::ThreadQueue& queue = *(ThreadSpike::ThreadQueue*)qptr;
  u8 bytes[THREADQUEUE_MAX_BYTES];

  bool readCorrectly = true;
  queue.Read(bytes, THREADQUEUE_MAX_BYTES);

  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES; i++)
  {
    if(i != bytes[i])
    {
      readCorrectly = false;
    }
  }
  if(!readCorrectly)
  {
    fprintf(stderr, "ERROR: TestNonBlockingRead failed.\n");
  }
  return (void*)readCorrectly;
}

bool TestNonBlockingRead()
{
  pthread_t producer, consumer;

  ThreadSpike::ThreadQueue queue;

  void* returnCorrectly;

  pthread_create(&producer, NULL, &ProducerWriteNonBlocking, &queue);
  Sleep(1,0);
  pthread_create(&consumer, NULL, &ConsumerReadNonBlocking, &queue);

  pthread_join(producer, NULL);
  pthread_join(consumer, &returnCorrectly);
  return (bool)returnCorrectly;
}

/***********************************************


             Blocking Test


/***********************************************/

void* ProducerWriteBlocking(void* qptr)
{
  ThreadSpike::ThreadQueue& queue = *(ThreadSpike::ThreadQueue*)qptr;
  const u32 chunkLength = 8;
  u8 writeBytes[chunkLength];

  
  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES / chunkLength; i++)
  {
    /* Quarter second sleeps */
    Sleep(0, 500000000);
    /* Fill with the next chunkLength bytes */
    for(u32 j = 0; j < chunkLength; j++)
    {
      writeBytes[j] = i * chunkLength + j;
    }
    queue.Write(writeBytes, chunkLength);
  }
  
  return NULL;
}

void* ConsumerReadBlocking(void* qptr)
{
  ThreadSpike::ThreadQueue& queue = *(ThreadSpike::ThreadQueue*)qptr;
  u8 readBytes[THREADQUEUE_MAX_BYTES];
  bool readCorrectly = true;
  queue.ReadBlocking(readBytes, THREADQUEUE_MAX_BYTES);

  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES; i++)
  {
    if(i != readBytes[i])
    {
      readCorrectly = false;
    }
  }
  if(!readCorrectly)
  {
    fprintf(stderr, "ERROR: TestBlockingRead failed.\n");
  }
  return (void*)readCorrectly;
}

bool TestBlockingRead()
{
  pthread_t producer, consumer;

  ThreadSpike::ThreadQueue queue;

  void* returnCorrectly;

  pthread_create(&producer, NULL, &ProducerWriteBlocking, &queue);
  pthread_create(&consumer, NULL, &ConsumerReadBlocking, &queue);

  pthread_join(producer, NULL);
  pthread_join(consumer, &returnCorrectly);
  return (bool)returnCorrectly;
}

#endif /* SPIKETESTS_H */
