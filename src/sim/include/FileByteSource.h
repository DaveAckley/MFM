/*                                              -*- mode:C++ -*-
  FileByteSource.h MFM ByteSource backed by a file
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
  \file FileByteSource.h MFM ByteSource backed by a file
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef FILEBYTESOURCE_H
#define FILEBYTESOURCE_H

#include <stdio.h>
#include "ByteSource.h"

namespace MFM
{
  /**
   * A ByteSource which uses a file descriptor as its reading source.
   */
  class FileByteSource : public ByteSource
  {
  private:
    /**
     * The file descriptor which backs this ByteSource. Is this is not
     * a valid file descriptor, this will always be NULL .
     */
    FILE* m_fp;

  public:
    /**
     * Constructs a new FileByteSource which is not ready for reading
     * from.  One must call \c Open() before using.
     */
    FileByteSource() :
      ByteSource(),
      m_fp(NULL)
    { }

    /**
     * Constructs a new FileByteSource which is ready for reading
     * from, given that the provided filename is a legal file to open.
     *
     * @param filename The path to the FILE that is wished to back
     *                 this FileByteSource and will therefore be read
     *                 from.
     */
    FileByteSource(const char* filename) :
      ByteSource(),
      m_fp(NULL)
    {
      Open(filename);
    }

    /**
     * Opens the file at a given path and allows this FileByteSource
     * to begin reading from it. If this FileByteSource is already
     * backed by an open file descriptor, this will do nothing. One
     * should check to see if this FileByteSource \c IsOpen() before
     * and after calling this method.
     *
     * @param filename The string representing the path to the file
     *        wished to back this FileByteSource .
     */
    void Open(const char* filename)
    {
      if(!m_fp)
      {
	m_fp = fopen(filename, "r");
      }
    }

    /**
     * Checks to see if this FileByteSource is backed by a valid file
     * descriptor and is therefore ready to write to.
     *
     * @returns \c true if this FileByteSource is ready to be written
     *          to, else \c false .
     */
    bool IsOpen()
    {
      return m_fp != NULL;
    }

    /**
     * Closes this FileByteSource, disallowing writing. One must call
     * \c Open() to use this FileByteSource after closing it.
     */
    void Close()
    {
      if(m_fp)
      {
	fclose(m_fp);
	m_fp = NULL;
      }
    }

    virtual int ReadByte()
    {
      if (!m_fp)
      {
        FAIL(ILLEGAL_STATE);
      }
      return fgetc(m_fp);
    }
  };
}

#endif /* FILEBYTESOURCE_H */
