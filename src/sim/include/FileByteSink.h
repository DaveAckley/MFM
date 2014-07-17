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

  public:

    /**
     * Constructs a new FileByteSink which writes bytes to a given
     * open file descriptor.
     *
     * @param file The FILE* wihch all bytes written to this
     *             FileByteSink are written to.
     */
    FileByteSink(FILE * file) :
      m_file(file)
    {
      if (!file)
      {
        FAIL(NULL_POINTER);
      }
    }

    virtual void WriteBytes(const u8 * data, const u32 len)
    {
      if(!m_file)
      {
	FAIL(NULL_POINTER);
      }
      size_t wrote = fwrite(data, 1, len, m_file);
      if (wrote != len)
        FAIL(IO_ERROR);
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
