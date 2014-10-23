/* -*- C++ -*- */
#include <stdlib.h>
#include "ThreadQueue.h"
#include "Fail.h"
#include "Logger.h"
#include "Util.h"

// Util.h has this #define MIN(X,Y) ((X) > (Y) ? (Y) : (X))

namespace MFM
{
  ThreadQueue::ThreadQueue() : m_moreInputIsAvailable(*this)
  {
    m_readHead = m_writeHead = m_heldBytes = 0;
  }

  ThreadQueue::~ThreadQueue()
  { }

  void ThreadQueue::Write(u8* bytes, u32 length)
  {
    if (length > 0)
    {
      Mutex::ScopeLock lock(m_mutex);

      if(length + m_heldBytes > THREADQUEUE_MAX_BYTES)
      {
        LOG.Error("ERROR: THREADQUEUE OVERLOAD!\n");
        FAIL(OUT_OF_RESOURCES);
      }

      for(u32 i = 0; i < length; i++)
      {
        m_queueData[m_writeHead++] = *(bytes++);
        if(m_writeHead >= THREADQUEUE_MAX_BYTES)
        {
          m_writeHead %= THREADQUEUE_MAX_BYTES;
        }
      }
      m_heldBytes += length;
      m_moreInputIsAvailable.SignalCondition();
    }
  }

  u32 ThreadQueue::UnsafeRead(u8* bytes, u32 length)
  {
    u32 bytesAvailable = MIN(length, m_heldBytes);
    for(u32 i = 0; i < bytesAvailable; i++)
    {
      *(bytes++) = m_queueData[m_readHead];
      m_queueData[m_readHead++] = 0xdb; // Mark as a dead byte
      if(m_readHead >= THREADQUEUE_MAX_BYTES)
      {
        m_readHead %= THREADQUEUE_MAX_BYTES;
      }
    }
    m_heldBytes -= bytesAvailable;
    return bytesAvailable;
  }

  void ThreadQueue::ReadBlocking(u8* bytes, u32 length)
  {
    Mutex::ScopeLock lock(m_mutex);
    u32 readBytes = 0;
    while(readBytes < length)
    {
      readBytes += UnsafeRead(bytes + readBytes, length - readBytes);
      if(readBytes < length)
      {
        m_moreInputIsAvailable.WaitForCondition();  // Release lock and wait for data
      }
    }
  }

  u32 ThreadQueue::Read(u8* bytes, u32 length)
  {
    u32 readBytes = 0;
    /* Bail if there aren't enough bytes to read. */
    /* We are checking m_heldBytes without holding the lock because
       writing to the queue can only make m_heldBytes larger, so if
       there's enough held bytes now, there still will be even if a
       write intervenes before we get the lock. */
    if(m_heldBytes >= length)
    {
      Mutex::ScopeLock lock(m_mutex);
      readBytes = UnsafeRead(bytes, length);
    }
    return readBytes;
  }

  void ThreadQueue::PeekRead(u8* toBuffer, u32 index, u32 length)
  {
    Mutex::ScopeLock lock(m_mutex);
    {
      if(m_heldBytes < length + index)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }

      u32 readHead = index + m_readHead;
      for(u32 i = 0; i < length; i++)
      {
        *(toBuffer++) = m_queueData[readHead++];
        if(readHead >= THREADQUEUE_MAX_BYTES)
        {
          readHead %= THREADQUEUE_MAX_BYTES;
        }
      }
    }
  }

  u32 ThreadQueue::BytesAvailable()
  {
    Mutex::ScopeLock lock(m_mutex);
    return m_heldBytes;
  }

  void ThreadQueue::Flush()
  {
    Mutex::ScopeLock lock(m_mutex);
    m_readHead = m_writeHead = m_heldBytes = 0;
  }
}
