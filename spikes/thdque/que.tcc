#include <stdio.h>    /* -*- C++ -*- */
#include <stdlib.h>

namespace ThreadSpike
{
  ThreadQueue::ThreadQueue()
  {
    m_readHead = m_writeHead = m_heldBytes = 0;
    pthread_mutex_init(&m_lock, NULL);    
  }
  
  ThreadQueue::~ThreadQueue()
  {
    pthread_mutex_destroy(&m_lock);
  }

  void ThreadQueue::Write(u8* bytes, u32 length)
  {
    pthread_mutex_lock(&m_lock);
    {
      if(length + m_heldBytes > THREADQUEUE_MAX_BYTES)
      {
	/* Won't worry too badly about the Fail macro here. */
	fprintf(stderr, "ERROR: THREADQUEUE OVERLOAD! \n");
	exit(1);
      }

      for(u32 i = 0; i < length; i++)
      {
	m_queueData[m_writeHead++] = *(bytes++);
	if(m_writeHead >= THREADQUEUE_MAX_BYTES)
	{
	  m_writeHead %= THREADQUEUE_MAX_BYTES;
	}
      }
    }
    pthread_mutex_unlock(&m_lock);
  }

  u32 ThreadQueue::Read(u8* bytes, u32 length)
  {
    u32 readBytes = 0;
    /* Bail if there aren't enough bytes to read. */
    if(m_heldBytes >= length)
    {
      pthread_mutex_lock(&m_lock);
      {
	for(u32 i = 0; i < length; i++)
	{
	  *(bytes++) = m_queueData[m_readHead++];
	  if(m_readHead >= THREADQUEUE_MAX_BYTES)
	  {
	    m_readHead %= THREADQUEUE_MAX_BYTES;
	  }

	}
      }
      pthread_mutex_unlock(&m_lock);
    }
    return readBytes;
  }

  u32 ThreadQueue::BytesAvailable()
  {
    return m_heldBytes;
  }
}
