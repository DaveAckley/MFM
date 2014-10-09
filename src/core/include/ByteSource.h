/*                                              -*- mode:C++ -*-
  ByteSource.h Abstract stream
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
  \file ByteSource.h Abstract stream
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef BYTESOURCE_H
#define BYTESOURCE_H

#include "itype.h"
#include "Fail.h"
#include "Format.h"
#include "ByteSink.h"
#include <stdarg.h>    /* For ... */

namespace MFM
{

  class ByteSerializable; // FORWARD

  /**
   * An abstract source where bytes may be read from.
   */
  class ByteSource
  {
  public:

    /**
     * Constructs a new ByteSource in an unread state.
     */
    ByteSource() :
      m_read(0),
      m_lastRead(-1),
      m_unread(false)
    { }

    /**
     * Gets the next logical single 32-bit character from this
     * ByteSource. If there was an \c Unread() operation on this
     * ByteSource before this \c Read() operation, the last character
     * returned by \c Read() will be returned.
     *
     * @returns the next logical single 32-bit character from this
     *          ByteSource.
     */
    s32 Read()
    {
      if (m_unread)
      {
	m_unread = false;
      }
      else
      {
	m_lastRead = ReadByte();
      }

      ++m_read;
      return m_lastRead;
    }

    /**
     * Puts the last read character back into this ByteSource. This is
     * useful for situations where the character on top of this
     * ByteSource is needed to be regurgitated; for instance, when
     * using the \c Peek() method.
     */
    void Unread()
    {
      if (m_unread)
      {
	FAIL(ILLEGAL_STATE);
      }
      m_unread = true;
      --m_read;
    }

    /**
     * Gets the next 32-bit character from the front of this
     * ByteSource without consuming it.
     *
     * @returns The next 32-bit character to be \c Read() from this
     *          ByteSource.
     */
    s32 Peek()
    {
      s32 read = Read();
      Unread();
      return read;
    }

    /**
     * Gets the next 32-bit character from this ByteSource , regardless
     * of whether or not \c Unread() has been called.
     *
     * @returns the next 32-bit character from this ByteSource .
     */
    virtual int ReadByte() = 0;

    /**
     * Deconstructs this ByteSource. Default implementation does nothing.
     */
    virtual ~ByteSource()
    { }

    /**
     * Gets the number of bytes that have currently been read from
     * this ByteSource. If a byte has been \c Unread() , it is treated
     * by this method as never having been read.
     *
     * @param The number of bytes that have currently been read from
     *        this ByteSource .
     */
    u32 GetBytesRead()
    {
      return m_read;
    }

    /**
     * Performs a binary read of the next 64 bits to come out of this
     * ByteSource . These bytes are stored in the specified u64 ,
     * where the first bytes read are stored in its MSB end.
     *
     * @param result The location to store the next 64 bits to come
     *        out of this ByteSource.
     *
     * @returns \c true if the next 64 bits are read successfully from
     *          this ByteSource, else \c false.
     */
    bool Scan(u64 & result) ;

    /**
     * Performs several types of reads of a 32-bit number, depending
     * on the provided format.
     *
     * @param result The location to store the read binary formatted
     *               32-bit number.
     *
     * @param code The format to treat the next bytes read from this
     *             ByteSource as, including various numeric bases
     *             (such as binary, octal, decimal, hexadecimal) and
     *             network-order binary representations.
     *
     * @param fieldWidth The maximum number of characters which will
     *                   be read during this call.
     *
     * @returns \c true if a number of \c code formatting can be read
     *          from the next read bytes.
     */
    bool Scan(s32 & result, Format::Type code = Format::DEC, u32 fieldWidth = U32_MAX) ;

    /**
     * Performs several types of reads of a 32-bit number, depending
     * on the provided format.
     *
     * @param result The location to store the read binary formatted
     *               32-bit number .
     *
     * @param code The format to treat the next bytes read from this
     *             ByteSource as, be it binary or ascii in nature.
     *
     * @param fieldWidth The maximum number of characters which will
     *                   be read during this call.
     *
     * @returns \c true if a number of \c code formatting can be read
     *          from the next read bytes.
     */
    bool Scan(u32 & result, Format::Type code = Format::DEC, u32 fieldWidth = U32_MAX) ;

    /**
     * Reads a ByteSerializable from the front of this ByteSource.
     *
     * @param byteSerializable The ByteSerializable which is to be
     *        expected during this Scan. The ByteSerializable itself
     *        defines whether or not these bytes may represent it.
     *
     * @param argument An argument which may be passed to \c
     *                 byteSerializable to help determine whether or
     *                 not is is being read correctly. Again, this is
     *                 determined by the particular ByteSerializable
     *                 used in this call.
     *
     * @return \c true if \c byteSerializable may be represented by
     *         the following read bytes, else \c false .
     */
    bool Scan(ByteSerializable & byteSerializable, s32 argument = 0) ;


    bool ScanLexDigits(u32 & digits) ;

    /**
     *
     */
    bool Scan(ByteSink & result, const u32 fieldWidth) ;

    s32 ScanSet(ByteSink & result, const char * setSpec)
    {
      return ScanSetFormat(result, setSpec);
    }

    /**
     * Scans a set of characters, defined by a specified descriptor,
     * consuming them as they are scanned.
     *
     * @param setSpec The descriptor of the characters to be consumed.
     *
     * @returns The number of characters consumed.
     */
    s32 SkipSet(const char * setSpec)
    {
      return ScanSet(DevNull, setSpec);
    }

    s32 ScanSetFormat(ByteSink & result, const char * & setSpec) ;

    /**
     * A char* consisting only of all unique characters treated as
     * Whitespace by this ByteSource. This set includes:
     *
     * ' '  Space
     * '\n' Newline
     * '\t' Tab
     * '\v' Vertical Tab
     */
    static const char * WHITESPACE_CHARS;

    /**
     * A char* representing a \c Scanf() formatted set of all chars in
     * \c WHITESPACE_CHARS . This is to be used when scanning for
     * whitespace.
     */
    static const char * WHITESPACE_SET;

    /**
     * A char* representing a \c Scanf() formatted set of all chars
     * which are not included in \c WHITESPACE_CHARS . This is to be
     * used when scanning for anything that is not whitespace.
     */
    static const char * NON_WHITESPACE_SET;

    /**
     * Scans this ByteSource for an Identifier, i.e. an alphanumeric
     * string which does not start with a number.
     *
     * @param result The ByteSink which collects the all characters
     *               that match the rules for being an identifier.
     *
     * @returns \c true if the next characters of this ByteSource
     *          (excluding whitespace) can be treated as an
     *          identifier.
     */
    bool ScanIdentifier(ByteSink & result)
    {
      SkipWhitespace();
      if (ScanSet(result, "[_a-zA-Z]") <= 0)
      {
        return false;
      }
      ScanSet(result, "[_a-zA-Z0-9]");
      return true;
    }

    /**
     * Scans this ByteSource for a hexadecimal string, i.e. a string
     * consisting of both upper and lowercase hexadecimal digits.
     *
     * @param result The ByteSink which collects the all characters
     *               that match the rules for being a hexadecimal
     *               string.
     *
     * @returns \c true if the next characters of this ByteSource
     *          (excluding whitespace) can be treated as a hexadecimal
     *          string.
     */
    bool ScanHex(ByteSink& result)
    {
      SkipWhitespace();

      return ScanSet(result, "[a-fA-F0-9]") > 0;
    }

    /**
     * Scans this ByteSource for a binary string, i.e. a string
     * consisting only of the binary characters '1' and '0' .
     *
     * @param result The ByteSink which collects the all characters
     *               that match the rules for being a binary string.
     *
     * @returns \c true if the next characters of this ByteSource
     *          (excluding whitespace) can be treated as a binary
     *          string.
     */
    bool ScanBinary(ByteSink& result)
    {
      SkipWhitespace();

      return ScanSet(result, "[0-1]") > 0;
    }

    /**
     * Scans this ByteSource for a camelcase identifier, i.e. an
     * alphanumeric string which starts with a capital letter.
     *
     * @param result The ByteSink which collects the all characters
     *               that match the rules for being an camelcase
     *               identifier.
     *
     * @returns \c true if the next characters of this ByteSource
     *          (excluding whitespace) can be treated as a camelcase
     *          identifier.
     */
    bool ScanCamelIdentifier(ByteSink & result)
    {
      SkipWhitespace();
      if (ScanSet(result, "[A-Z]") <= 0)
      {
        return false;
      }
      ScanSet(result, "[a-zA-Z0-9]");
      return true;
    }

    /**
     * Consumes and destroys all whitespace characters at the front of
     * this ByteSource.
     *
     * @returns The number of whitespace characters consumed.
     */
    s32 SkipWhitespace()
    {
      return SkipSet(WHITESPACE_SET);
    }

    s32 Scanf(const char * format, ...) ;  // NYI
    s32 Vscanf(const char * format, va_list & ap) ;

  private:
    s32 ReadCounted(u32 & maxLen)
    {
      if (maxLen == 0)
      {
        return -1;
      }

      s32 ret = Read();
      if (ret >= 0)
      {
        --maxLen;
      }
      return ret;
    }

    u32 m_read;
    s32 m_lastRead;
    bool m_unread;
  };

}

#endif /* BYTESOURCE_H */
