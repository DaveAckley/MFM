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
#include "Format.h"
#include <stdarg.h>    /* For ... */

namespace MFM {

  class ByteSource;   // FORWARD
  class ByteSerializable;   // FORWARD

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

    /**
     * [OPTIONAL OPERATION] If the last byte currently in this
     * ByteSink is a newline, remove that last byte, shortening the
     * content by one byte.  Returns \c true iff the content was
     * shortened.
     *
     * \fails UNSUPPORTED_OPERATION if this ByteSink does not
     * implement it.
     *
     * \sa CanChomp
     */
    virtual bool Chomp(s32 toChomp = '\n')
    {
      FAIL(UNSUPPORTED_OPERATION);
    }

    /**
     * Return \c true iff this ByteSink implements the \c Chomp
     * operation.
     */
    virtual bool CanChomp()
    {
      return false;
    }

    /**
       Base class destructor does nothing virtually
     */
    virtual ~ByteSink() { }

    /**
       Copy to this ByteSink any bytes remaining in restOfThis,
       leaving restOfThis at EOF.
     */
    void Copy(ByteSource & restOfThis) ;


    void Print(const char * str, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(const u8 * str, u32 len, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s32 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u32 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s64 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u64 decimal, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u32 num, Format::Type code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s32 num, Format::Type code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(u64 num, Format::Type code, s32 fieldWidth = -1, u8 padChar = ' ');
    void Print(s64 num, Format::Type code, s32 fieldWidth = -1, u8 padChar = ' ');

    void Print(ByteSerializable & byteSerializble, s32 argument = 0) ;

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
    void Println(u32 decimal, Format::Type code) ///< \eq #Print(u32 decimal, Format::Type code) followed by #Println().
    {
      Print(decimal,code);
      Println();
    }

    void Println(ByteSerializable & byteSerializable, s32 argument = 0) ;

    void Printf(const char * format, ...) ;

    void PrintAbbreviatedNumber(u64 number) ;

    void Vprintf(const char * format, va_list & ap) ;

    void PrintDoubleQuotedString(const char * zstring) ;

    /**
       Print a string in which the length (<=255) of the string is
       stored at nstring[-1], using C-style escaping for non-printing
       chars and " and \
     */
    void PrintDoubleQuotedCStringWithLength(const char * nstring) ;

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

  /**
     A class for a ByteSink that can be 'Reset' (so it can be written again).
   */
  class ResettableByteSink : public ByteSink
  {
  public:
    /** 
      A method that 'resets' this ByteSink in some sensible fashion,
      depending on its specifics, so that it can be written to 'as if
      new'
    */
    virtual void Reset() = 0;
  };

  /**
     A ByteSink that can be converted to a ZString, and reset to
     empty, in some sensible way.  Note that this interface does \e
     not promise any strong functional constraints among Reset, Printf
     (say, or any other writing method), and GetZString.
   */
  class ZStringableByteSink : public ResettableByteSink
  {
  public:

    /**
     * Resets this ZStringableByteSink back to its initial state.
     */
    virtual void Reset() = 0;

    /**
     * Gets the current contents of this ZStringableByteSink
     */
    virtual const char * GetZString() const = 0;

  };

  class DiscardBytes : public ByteSink {
  public:

    virtual void WriteBytes(const u8 * data, const u32 len)
    { }

    virtual s32 CanWrite()
    {
      return S32_MAX;
    }

  };

  extern DiscardBytes DevNullByteSink;

  // DON'T KNOW WTH TO STICK THESE:
  extern void PrintBacktrace(FILE * f, void * const * backtraceArray, unsigned backtraceSize) ;
  extern void LogBacktrace(void * const * backtraceArray, unsigned backtraceSize) ;
  extern void DumpBacktrace(ByteSink & bt, void * const * backtraceArray, unsigned backtraceSize) ;
}

#include "ByteSink.tcc"

#endif /* BYTESINK_H */
