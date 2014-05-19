/* -*- C++ -*- */
#ifndef FILEBYTESINK_H
#define FILEBYTESINK_H

#include "itype.h"
#include <stdio.h>
#include "ByteSink.h"

namespace MFM
{
  class FileByteSink : public ByteSink {
    FILE * m_file;
  public:

    FileByteSink(FILE * file) : m_file(file) {
      if (!file)
        FAIL(NULL_POINTER);
    }

    virtual void WriteBytes(const u8 * data, const u32 len) {
      size_t wrote = fwrite(data, 1, len, m_file);
      if (wrote != len)
        FAIL(IO_ERROR);
    }

    virtual s32 CanWrite() {
      return 1;  // XXX WHAT GOES HERE FFS?
    }
  };

  extern FileByteSink STDOUT;
  extern FileByteSink STDERR;
}

#endif /* FILEBYTESINK_H */
