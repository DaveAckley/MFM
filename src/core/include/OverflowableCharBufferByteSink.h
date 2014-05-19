/* -*- C++ -*- */
#ifndef OVERFLOWABLECHARBUFFERBYTESINK_H
#define OVERFLOWABLECHARBUFFERBYTESINK_H

#include "ByteSink.h"
#include <string.h>        /* For memcpy */

namespace MFM {

  template <int BUFSIZE>
  class OverflowableCharBufferByteSink : public ByteSink {
  public:
    OverflowableCharBufferByteSink() : m_written(0), m_overflowed(false) { }

    virtual void WriteBytes(const u8 * data, const u32 len) {

      if (m_overflowed)
        return;

      if (m_written + len < BUFSIZE - 2) {
        memcpy(&m_buf[m_written], data, len);
        m_written += len;
        return;
      }

      m_buf[m_written++] = 'X';
      m_overflowed = true;
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
      m_overflowed = false;
    }

  private:
    u8 m_buf[BUFSIZE];
    u32 m_written;
    bool m_overflowed;
  };
}

#endif /* OVERFLOWABLECHARBUFFERBYTESINK_H */
