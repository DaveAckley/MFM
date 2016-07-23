#include "ByteSource.h"
#include "ByteSerializable.h"
#include "BitVector.h"
#include <ctype.h>  /* for tolower */

namespace MFM {

  bool ByteSource::Scan(u64 & result, Format::Type code)
  {
    u32 base = 10;
    switch (code)
    {
    case Format::BEU64:
      {
        u64 num = 0;
        for (u32 i = 0; i < 8; ++i)
        {
          s32 ch = Read();
          if (ch < 0)
            return false;
          num = (num << 8) | ch;
        }
        result = num;
      }
      return true;

    case Format::LXX64:
      base = 16;
      // FALL THROUGH
    case Format::LEX64:
      {
        u32 len;
        if (!ScanLexDigits(len)) return false;

        u64 num = 0;
        for (u32 i = 0; i < len; ++i)
        {
          s32 ch = Read();
          if (ch < 0)
            return false;
          u8 uch = (u8) tolower(ch);
          u32 dig;
          if (uch >= '0' && uch <= '9')
            dig = uch-'0';
          else if (uch >= 'a' && uch < 'a'+base-10)
            dig = uch-'a'+10;
          else
          {
            Unread();
            return false;
          }
          num = (num * base) + dig;
        }
        result = num;
      }
      return true;

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  bool ByteSource::Scan(s32 & result, Format::Type code, u32 fieldWidth)
  {
    s32 ch;
    switch (code) {
    case Format::BEU64:
    case Format::LEX64:
    case Format::LXX64:
      FAIL(ILLEGAL_ARGUMENT);    // Can't 64 into a 32 doh

    case Format::BYTE:
    case Format::BEU16:
    case Format::BEU32: {        // Handle raw formats
      s32 num = 0;
      for (int i = 1 << -((s32) code); i > 0; --i) {
        if ((ch = Read()) < 0)   // Raw formats ignore fieldWidth
          return false;
        num = (num << 8) | ch;
      }
      result = num;
      return true;
    }

    case Format::LEXHD:         // Self-delimited formats ignore fieldWidth
      return ScanLexDigits(*(u32*)&result);
      break;

    case Format::LEX32:
    case Format::LXX32: {
      u32 base = 10;
      if (code==Format::LXX32)
        base = 16;

      u32 len;
      if (!ScanLexDigits(len)) return false;
      return Scan(result, (Format::Type) base, len);
    }

    default: // Continue analysis below
      break;
    }

    // Here to read in base 2..36
    MFM_API_ASSERT_ARG(code >= 2 && code <= 36);

    // Skip leading spaces (only, not other 'whitespace')
    do {
      if ((ch = ReadCounted(fieldWidth)) < 0)
        return false;           // Can't get started
    } while (ch == ' ');

    // ch is now a non-whitespace
    bool negative;
    if ((negative = ch=='-') || ch=='+') {
      if ((ch = ReadCounted(fieldWidth)) < 0)
        return false;           // Can't get started
    }

    // We are now holding a read-but-unaccepted char in ch
    int num = 0;
    bool readSome = false;
    do {
      bool valid = true;
      u8 val;
      if (ch >= '0' && ch <= '9') val = ch-'0';
      else {
        ch |= 0x20;  // standardize on lowercase
        if (ch >= 'a' && ch <= 'z') val = ch-('a'-10);
        else
          valid = false;
      }
      if (valid && val < code) {
        num = num * code + val;
        readSome = true;
      } else {
        Unread();               // We read something we didn't want, back up
        if (readSome) break;    // If we saw something okay before, okay
        else return false;      // Otherwise this is messed up
      }
    } while ((ch = ReadCounted(fieldWidth)) >= 0);
    result = negative?-num:num;
    return true;
  }

  bool ByteSource::Scan(u32 & result, Format::Type code, u32 maxLen)
  {
    return Scan(*(s32*)&result, code, maxLen);
  }

  bool ByteSource::Scan(ByteSerializable & byteSerializable, s32 argument)
  {
    ByteSerializable::Result res = byteSerializable.ReadFrom(*this, argument);
    MFM_API_ASSERT(res != ByteSerializable::UNSUPPORTED, UNSUPPORTED_OPERATION);
    return res == ByteSerializable::SUCCESS;
  }

  bool ByteSource::ScanLexDigits(u32 & digits)
  {
    u32 byte;
    if (!Scan(byte, Format::BYTE))
    {
      return false;
    }

    if (byte >= '0' && byte <= '8')
    {
      digits = byte-'0';
      return true;
    }

    if (byte =='9')
    {
      return Scan(digits, Format::LEX32);
    }
    return false;
  }

  bool ByteSource::Scan(ByteSink & result, const u32 fieldWidth)
  {
    for (u32 i = 0; i < fieldWidth; ++i) {
      s32 ch = Read();
      if (ch < 0) return false;
      result.WriteByte(ch);
    }
    return true;
  }


  s32 ByteSource::ScanSetFormat(ByteSink & result, const char * & setSpec)
  {
    MFM_API_ASSERT_NONNULL(setSpec);
    MFM_API_ASSERT(*setSpec++ == '[', BAD_FORMAT_ARG);

    typedef BitVector<256> CharMap;
    CharMap map;
    bool excluded = false;

    if (*setSpec == '^') {
      excluded = true;
      ++setSpec;
    }

    s32 prevCh = -1;  // no prev
    bool complete = false;
    for (s32 ch; (ch = *setSpec++) > 0; prevCh = ch) {
      if (ch == ']') {

        if (prevCh < 0)
          map.SetBit((u32) ch);
        else {
          complete = true;
          break;
        }

      } else if (ch == '-') {

        // If no prev, or end is next
        s32 next = *setSpec;
        if (prevCh < 0 || next == ']')     // Then '-' is not special
          map.SetBit((u32) ch);
        else {
          MFM_API_ASSERT(prevCh <= next, BAD_FORMAT_ARG);
          map.SetBits(prevCh, next - prevCh + 1);

          prevCh = next;  // Apparently [a-b-e] matches 'c', so..
          ++setSpec;

        }
      } else map.SetBit((u32) ch);
    }

    MFM_API_ASSERT(complete, BAD_FORMAT_ARG);

    s32 count = 0;
    s32 ch;
    while ((ch = Read()) >= 0) {
      if (excluded == map.ReadBit(ch)) {
        Unread();
        return count;
      }
      result.WriteByte(ch);
      ++count;
    }
    if (ch < 0 && count == 0)
      return ch;
    return count;
  }

  const char * ByteSource::WHITESPACE_CHARS = " \n\t\v";
  const char * ByteSource::WHITESPACE_SET = "[ \n\t\v]";
  const char * ByteSource::NON_WHITESPACE_SET = "[^ \n\t\v]";

  s32 ByteSource::Scanf(const char * format, ...)
  {
    va_list ap;
    va_start(ap, format);
    s32 ret = Vscanf(format, ap);
    va_end(ap);
    return ret;
  }

  static void StorePtr8(va_list & ap, u64 val) {
    u64 * ptr = va_arg(ap,u64*);
    if (!ptr) return;
    MFM_API_ASSERT_ZERO(((uptr)ptr)&7);
    *ptr = val;
  }

  static void StorePtr4(va_list & ap, u32 val) {
    u32 * ptr = va_arg(ap,u32*);
    if (!ptr) return;
    MFM_API_ASSERT_ZERO(((uptr)ptr)&3);
    *ptr = val;
  }

  static void StorePtr2(va_list & ap, u16 val) {
    u16 * ptr = va_arg(ap,u16*);
    if (!ptr) return;
    MFM_API_ASSERT_ZERO(((uptr)ptr)&1);
    *ptr = val;
  }

  static void StorePtr1(va_list & ap, u8 val) {
    u8 * ptr = va_arg(ap,u8*);
    if (!ptr) return;
    *ptr = val;
  }



  s32 ByteSource::Vscanf(const char * format, va_list & ap)
  {
    MFM_API_ASSERT_NONNULL(format);

    u8 p;
    s32 result;
    s32 fieldWidth;
    /// NYI: u8 padChar;

    bool alt;
    int matches = 0;
    while ((p = *format++)) {
      if (p != '%') {
        result = Read();
        if (result < 0)
          return matches;
        if ((u8) result != p)
          return matches;
        ++matches;
        continue;
      }

      // Here we read an '%'
      alt = false;
      fieldWidth = -1;
      s32 pointerSize = -1;
      //NYI: padChar = ' ';

    again:
      u32 type = 0;
      switch (p = *format++) {
      case '?':
        if (pointerSize >= 0) FAIL(ILLEGAL_ARGUMENT);
        pointerSize = va_arg(ap,s32);
        goto again;

      case '#': alt = true; goto again;

      case '0':
        if (fieldWidth < 0) {
          //NYI: padChar = '0';
          fieldWidth = 0;
        } else fieldWidth *= 10;
        goto again;

      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
        if (fieldWidth < 0) fieldWidth = 0;
        fieldWidth = fieldWidth * 10 + (p - '0');
        goto again;

      case 'c':
        if (!Scan(result, Format::BYTE))
          return -matches;
        StorePtr1(ap, (u8) result);
        ++matches;
        break;

      case 'h':
        if (!Scan(result, Format::BEU16))
          return -matches;
        StorePtr2(ap, (u16) result);
        ++matches;
        break;

      case 'l':
        if (!Scan(result, Format::BEU32))
          return -matches;
        StorePtr4(ap, (u32) result);
        ++matches;
        break;

      case 'q':
        {
          u64 res64;
          if (!Scan(res64,Format::BEU64)) return -matches;
          StorePtr8(ap, res64);
          ++matches;
        }
        break;

      // %w is optional whitespace: counts as match even if none, consumes no arg
      case 'w':
        SkipWhitespace();
        ++matches;
        break;

      // %W is mandatory whitespace: fails unless at least one, consumes no arg
      case 'W':
        if (SkipWhitespace() == 0)
          return -matches;
        ++matches;
        break;

        // %z to match a zero-terminated string.  Must match entirely,
        // if it does, it counts as one match regardless of its
        // length.  If it doesn't match, caller isn't told how far
        // through the string the scanner got.
      case 'z':
        {
          const u8 * zstr = va_arg(ap,const u8 *);
          MFM_API_ASSERT_NONNULL(zstr);
          u8 ch;
          while ((ch = *zstr++)) {
            result = Read();
            if (result < 0)
              return matches;
            if ((u8) result != ch)
              return -matches;
          }
          ++matches;
        }
        break;

        // %Z to find a zero-terminated string on the input, and store
        // it, minus the terminating null, in the ByteSink* arg.
        // Counts as one match regardless of its length, unless EOF is
        // encountered before a null is found, in which case no match
        // is counted, but any non-null bytes encountered will have
        // been written to arg.  %#Z is the same but does not consume
        // an argument, and discards bytes rather than writing them.
      case 'Z': 
        {
          ByteSink * bs;
          if (alt)
            bs = &DevNullByteSink;
          else
            bs = va_arg(ap,ByteSink*);
          
          while (true)
          {
            result = Read();
            if (result <= 0) {
              if (result == 0) ++matches;
              break;
            }
            bs->WriteByte(result);
          }

          if (result < 0) 
            return -matches;
        }
        break;

      case 'b': type = Format::BIN; goto store;
      case 'o': type = Format::OCT; goto store;
      case 'd': type = Format::DEC; goto store;
      case 'x': type = Format::HEX; goto store;
      case 't': type = Format::B36; goto store;

      case 'D': type = Format::LEX32; goto store;
      case 'X': type = Format::LXX32; goto store;

      store:
        if (!Scan(result,(Format::Type) type, fieldWidth))
          return -matches;
        switch (pointerSize)
        {
        case -1:
        case 4: StorePtr4(ap, (u32) result); break;
        case 2: StorePtr2(ap, (u16) result); break;
        case 1: StorePtr1(ap, (u8) result); break;
        default: FAIL(ILLEGAL_STATE);
        }
        ++matches;
        break;

      case '@':
        {
          s32 argument = 0;
          if (alt) argument = va_arg(ap,s32);
          ByteSerializable * bs = va_arg(ap,ByteSerializable*);
          MFM_API_ASSERT_NONNULL(bs);
          if (bs->ReadFrom(*this, argument) != ByteSerializable::SUCCESS)
            return -matches;
          ++matches;
        }
        break;

      case '[':
        {
          --format;  // Back pointer up to '['
          ByteSink * bs = 0;
          if (!alt)
            bs = va_arg(ap,ByteSink*);
          if (!bs) bs = &DevNullByteSink;
          s32 res = ScanSetFormat(*bs, format); // Advances format to ']' or FAILs
          if (res < 0)
            return -matches;
          if (res == 0 && fieldWidth != 0) // Special case: Allow %0[set] to match nothing
            return -matches;
          ++matches;
        }
        break;

      case '%':
        if (!Scan(result,Format::BYTE)) return -matches;
        if (result != '%') return -matches;
        ++matches;
        break;

      default:                  // Either I don't know that code, or you're bogus.
        FAIL(BAD_FORMAT_ARG);   // Either way, I die.  You're welcome.
      }
    }
    return matches;
  }
}
