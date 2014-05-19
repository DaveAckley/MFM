/* -*- C++ -*- */
#ifndef CHARBUFFERBYTESINK_H
#define CHARBUFFERBYTESINK_H

#include "ByteSink.h"
#include <string.h>        /* For memcpy */

namespace MFM {

  template <int BUFSIZE>
  class CharBufferByteSink : public ByteSink {
  public:
    CharBufferByteSink() : m_written(0) { }
    virtual void WriteBytes(const u8 * data, const u32 len) {
      if (m_written + len < BUFSIZE-1) {
        memcpy(&m_buf[m_written], data, len);
        m_written += len;
        return;
      }
      FAIL(OUT_OF_ROOM);
    }

    virtual s32 CanWrite() {
      return BUFSIZE - m_written - 1;
    }

    bool Equals(const char * str) {
      return strcmp(GetZString(), str)==0;
    }
    const char * GetZString() {
      m_buf[m_written] = '\0';
      return (const char *) m_buf;
    }

    u32 GetLength() const {
      return m_written;
    }

    u32 GetCapacity() const {
      return BUFSIZE;
    }

    void Reset() {
      m_written = 0;
    }

  private:
    u8 m_buf[BUFSIZE];
    u32 m_written;
  };
}

#endif /* CHARBUFFERBYTESINK_H */
