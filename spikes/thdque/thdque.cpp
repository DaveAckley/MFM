#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "itype.h"
#include "que.h"

void* ProducerWrite(void* qptr)
{
  ThreadSpike::ThreadQueue queue = *(ThreadSpike::ThreadQueue*)qptr;
  u8 writeBytes[THREADQUEUE_MAX_BYTES];
  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES; i++)
  {
    writeBytes[i] = i;
  }
  queue.Write(writeBytes, THREADQUEUE_MAX_BYTES);
  return NULL;
}

void* ConsumerRead(void* qptr)
{
  ThreadSpike::ThreadQueue queue = *(ThreadSpike::ThreadQueue*)qptr;
  u8 bytes[THREADQUEUE_MAX_BYTES];
  queue.Read(bytes, THREADQUEUE_MAX_BYTES);

  for(u32 i = 0; i < THREADQUEUE_MAX_BYTES; i++)
  {
    if(i != bytes[i])
    {
      fprintf(stderr, "ERROR: Byte invalid: %d != %d\n", i, bytes[i]);
    }
  }
  return NULL;
}

int main(int argc, char** argv)
{
  pthread_t producer, consumer;

  u8 writeBytes[THREADQUEUE_MAX_BYTES];

  ThreadSpike::ThreadQueue queue;

  pthread_create(&producer, NULL, &ProducerWrite, &queue);
  sleep(2);
  pthread_create(&consumer, NULL, &ConsumerRead, &queue);

  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);
  
  return 0;
}
