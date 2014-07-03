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
  class FileByteSource : public ByteSource
  {
  public:

    FileByteSource() : ByteSource(),
		       m_fp(NULL)
    { }

    FileByteSource(const char* filename) : ByteSource(),
					   m_fp(NULL)
    {
      Open(filename);
    }

    void Open(const char* filename)
    {
      if(!m_fp)
      {
	m_fp = fopen(filename, "r");
      }
    }

    void Close()
    {
      if(m_fp)
      {
	fclose(m_fp);
      }
    }

    virtual int ReadByte()
    {
      return fgetc(m_fp);
    }

  private:

    FILE* m_fp;
  };
}

#endif /* FILEBYTESOURCE_H */
