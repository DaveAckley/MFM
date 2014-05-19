#ifndef IO_H          /* -*- C++ -*- */
#define IO_H

#include "itype.h"
#include "Fail.h"
#include <stdarg.h>    /* For ... */

namespace MFM {

  class ByteSource {
  public:
    ByteSource() : m_read(0), m_lastRead(-1), m_unread(false) { }

    s32 Read() {
      if (m_unread) m_unread = false;
      else m_lastRead = ReadByte();

      ++m_read;
      return m_lastRead;
    }

    void Unread() {
      if (m_unread) FAIL(ILLEGAL_STATE);
      m_unread = true;
      --m_read;
    }

    s32 Peek() {
      s32 read = Read();
      Unread();
      return read;
    }

    virtual int ReadByte() = 0;
    virtual ~ByteSource() { }

    u32 GetBytesRead() {
      return m_read;
    }

    s32 Scanf(const char * format, ...) ;
    s32 Vscanf(const char * format, va_list & ap) ;

  private:
    u32 m_read;
    s32 m_lastRead;
    bool m_unread;
  };

}

#endif /* IO_H */
