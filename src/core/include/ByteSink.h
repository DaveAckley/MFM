/*                                              -*- mode:C++ -*-
  ByteSink.h Stream output support
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
  \file ByteSink.h Stream output support
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef BYTESINK_H
#define BYTESINK_H

#include "itype.h"
#include "Fail.h"
#include <stdarg.h>    /* For ... */

namespace MFM {

  class ByteSink; // FORWARD

  /**
     ByteSinkable is an interface for an object that can print itself
     to a ByteSink.
   */
  class ByteSinkable {
  public:
    /**
       Prints a representation of the ByteSinkable to the given \a
       byteSink, with its details possibly modified by \a argument.
       The meaning of \a argument is unspecified except that 0 should
       be interpreted as a request for default, 'no modification',
       formatting.
     */
    virtual void PrintTo(ByteSink & byteSink, s32 argument = 0) = 0;

    virtual ~ByteSinkable() { }
  };

  /**
     ByteSink is an abstract base class for byte stream output
     mechanisms.  Concrete instantiations of ByteSink must implement
     two methods: \ref WriteBytes to output a given sequence of bytes,
     in its entirety, blocking if need be to do so, and \ref CanWrite
     to return the minimum number of bytes that can currently be
     written via WriteBytes without having it block.
   */
  class ByteSink {
  public:

    /**
       Write the \a len consecutive bytes starting at \a data to the
       ByteSink, blocking if and as long as necessary to do so.  It is
       not an error for \a len to be zero and WriteBytes will never
       block if it is.

       \fails OUT_OF_ROOM if there is permanently not enough room to
       perform the requested output

       \fails IO_ERROR if some other unrecoverable output error occurs.

       \sa CanWrite
     */
    virtual void WriteBytes(const u8 * data, const u32 len) = 0;


    /**
       Return the number of bytes that can currently be written by
       \ref WriteBytes without blocking, or return a negative value if
       the ByteSink has encountered an error.

       XXX DOCUMENT ERROR VALUES
     */
    virtual s32 CanWrite() = 0;

    /**
       Overridable convenience method \eq
       \code
       WriteBytes(&ch, 1);
       \endcode
     */
    virtual void WriteByte(u8 ch) {
      WriteBytes(&ch, 1);
    }

    /**
       Overridable convenience method \eq
       \code
       WriteByte('\n');
       \endcode
     */
    virtual void WriteNewline() {
      WriteByte('\n');
    }

    virtual ~ByteSink() { }

    /**
       Formatting codes for use with \ref Print(u32, BaseCode), \ref
       Print(s32, BaseCode), and \ref Print(u64, BaseCode).
     */
    enum BaseCode {

      DEC = 10,      /**< Print in decimal, signed if negative (in most contexts) */
      HEX = 16,      /**< Print in hexadecimal (base 16, using 0-9A-Z), unsigned */
      OCT = 8,       /**< Print in octal (base 8, using 0-7), unsigned */
      BIN = 2,       /**< Print in binary (base 2, using 0-1), unsigned */
      BYTE = 0,      /**< Print a single byte directly, as an unaltered 8 bit value */

      BEU16 = -1,    /**< Print a u16 (unsigned 2 bytes) in big endian (network order) */
      BEU32 = -2,    /**< Print a u32 (unsigned 4 bytes) in big endian (network order) */
      BEU64 = -3,    /**< Print a u64 (unsigned 8 bytes) in big endian (network order) */
      LEX32 = -4,    /**< Print a u32 (unsigned 4 bytes) in leximited decimal */
      LXX32 = -5,    /**< Print a u32 (unsigned 4 bytes) in leximited hexadecimal */
      LEX64 = -6,    /**< Print a u64 (unsigned 8 bytes) in leximited decimal */
      LXX64 = -7,    /**< Print a u64 (unsigned 8 bytes) in leximited hexadecimal */
      LEXHD = -8,    /**< Print a u32 (unsigned 4 bytes) as a leximited format header */
      B36 = 36       /**< Print in base 36 ("hexatrigintimal"?, using 0-9A-Z), unsigned */
    };

    void Print(const char * str, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(const u8 * str, u32 len, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s32 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u32 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s64 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u64 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u32 num, BaseCode code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s32 num, BaseCode code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u64 num, BaseCode code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s64 num, BaseCode code, s32 fieldWidth = -1, u8 padChar = ' ');

    void Print(ByteSinkable & byteSinkable, s32 argument = 0)
    {
      byteSinkable.PrintTo(*this, argument);
    }

    void Println()
    {
      WriteNewline();
    }

    void Println(u8 byte)
    {
      Print(byte);
      Println();
    }
    void Println(const char * str)          ///< \eq #Print(const char * str) followed by #Println().
    {
      Print(str);
      Println();
    }
    void Println(const u8 * str, u32 len)  ///< \eq #Print(const u8 * str,u32 len) followed by #Println().
    {
      Print(str,len);
      Println();
    }
    void Println(s32 decimal)              ///< \eq #Print(s32 decimal)followed by #Println().
    {
      Print(decimal);
      Println();
    }
    void Println(u32 decimal)              ///< \eq #Print(u32 decimal) followed by #Println().
    {
      Print(decimal);
      Println();
    }
    void Println(u32 decimal, BaseCode code) ///< \eq #Print(u32 decimal, BaseCode code) followed by #Println().
    {
      Print(decimal,code);
      Println();
    }

    void Println(ByteSinkable & byteSinkable, s32 argument = 0) {
      Print(byteSinkable, argument);
      Println();
    }

    void Printf(const char * format, ...) ;

    void Vprintf(const char * format, va_list & ap) ;

  private:
    template <class UNSIGNED_TYPE>
    static u32 CountDigits(UNSIGNED_TYPE num, u32 base) {
      u32 digits = 0;
      do {
        ++digits;
        num /= base;
      } while (num > 0);
      return digits;
    }

    bool IsPrint(u32 ch) { return ch >= ' ' && ch <= '~';  }

    void PrintLexDigits(u32 digits) ;

    template <class UNSIGNED_TYPE>
    void PrintInBase(UNSIGNED_TYPE num, u32 base, s32 width = 0, u8 pad = ' ') ;
  };

  class DiscardAndDiscardable : public ByteSink, public ByteSinkable {
  public:
    virtual void PrintTo(ByteSink & byteSink, s32 argument)
    { }

    virtual void WriteBytes(const u8 * data, const u32 len)
    { }

    virtual s32 CanWrite()
    {
      return S32_MAX;
    }

  };

  extern DiscardAndDiscardable DevNull;

}

#include "ByteSink.tcc"

#endif /* BYTESINK_H */
