/*                                              -*- mode:C++ -*-
  FileByteSink.h Byte sink backed by a FILE*
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file FileByteSink.h Byte sink backed by a FILE*
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef FILEBYTESINK_H
#define FILEBYTESINK_H

#include "itype.h"
#include <stdio.h>
#include "ByteSink.h"

namespace MFM
{
  /**
   * A ByteSink that wraps a stdio.h FILE * pointer for disk file
   * output.
   */
  class FileByteSink : public ByteSink
  {
  private:
    /**
     * The FILE* which backs this FileByteSink. All writing is done
     * through this descriptor.
     */
    FILE * m_file;
    bool m_lineBuffered;  //< If true, flush after each \n written

  public:

    /**
     * Constructs a new FileByteSink which writes bytes to a given
     * open file descriptor.
     *
     * @param file The FILE* wihch all bytes written to this
     *             FileByteSink are written to.
     *
     * @param buffered If true, automatically flush this FileByteSink
     *             after each '\n' is printed. (Optional second
     *             argument, default false.)
     */
    FileByteSink(FILE * file, bool buffered = false)
      : m_file(file)
      , m_lineBuffered(buffered)
    {
      if (!file)
      {
        FAIL(NULL_POINTER);
      }
    }

    /** Determines if this FileByteSink is currently 'line buffered',
     *  meaning it will automatically flush after each '\n' is
     *  printed.
     */
    bool IsLineBuffered() const { return m_lineBuffered; }

    /** Set the line buffering on this FileByteSink.
     *
     * @param doLineBuffering true if this FileByteSink should now be
     *        line buffering
     */
    void SetLineBuffered(bool doLineBuffering) { m_lineBuffered = doLineBuffering; }

    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      if(!m_file)
      {
	FAIL(NULL_POINTER);
      }
      if (!m_lineBuffered)
      {
        size_t wrote = fwrite(data, 1, len, m_file);
        if (wrote != len)
          FAIL(IO_ERROR);
      }
      else
      {
        for (u32 i = 0; i < len; ++i)
        {
          u8 ch = data[i];
          size_t wrote = fwrite(&ch, 1, 1, m_file);
          if (wrote != 1)
            FAIL(IO_ERROR);
          if (ch == '\n')
            fflush(m_file);
        }
      }
    }

    /**
     * Closes the FILE* backing this FileByteSink. This FileByteSink
     * will no longer be able to be used after this is called.
     */
    void Close()
    {
      fclose(m_file);
      m_file = NULL;
    }

    virtual s32 CanWrite()
    {
      return (m_file != NULL) ? 1 : -1; /* Can't write to a closed stream */
    }
  };

  extern FileByteSink STDOUT;
  extern FileByteSink STDERR;
}

#endif /* FILEBYTESINK_H */
