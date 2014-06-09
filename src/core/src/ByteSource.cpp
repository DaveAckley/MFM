#include "ByteSource.h"
#include "BitVector.h"

namespace MFM {

  const char * ByteSource::WHITESPACE_CHARS = " \n\t\v";
  const char * ByteSource::WHITESPACE_SET = "[ \n\t\v]";
  const char * ByteSource::NON_WHITESPACE_SET = "[^ \n\t\v]";

  bool ByteSource::Scan(u32 & result, Format::Type code, u32 maxLen)
  {
    return Scan(*(s32*)&result, code, maxLen);
  }

  bool ByteSource::Scan(s32 & result, Format::Type code, u32 maxLen)
  {
    s32 ch;
    if (code <= Format::BYTE && code >= Format::BEU64) {  // Handle raw formats
      if (code == Format::BEU64)
        FAIL(ILLEGAL_ARGUMENT);           // Can't 64 into a 32 doh
      s32 num = 0;
      for (int i = 1 << -((s32) code); i > 0; --i) {
        if ((ch = Read()) < 0) return false;
        num = (num << 8) | ch;
      }
      result = num;
      return true;
    }

    // Here to read in base 2..36
    if (code < 2 || code > 36)
      FAIL(ILLEGAL_ARGUMENT);

    // Skip leading spaces (only, not other 'whitespace')
    do {
      if ((ch = Read()) < 0) return false;            // Can't get started
    } while (ch == ' ');

    // ch is now a non-whitespace
    bool negative;
    if ((negative = ch=='-') || ch=='+') {
      if ((ch = Read()) < 0) return false;            // Can't get started
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
    } while ((ch = Read()) >= 0);
    result = negative?-num:num;
    return true;
  }

  s32 ByteSource::ScanSetFormat(ByteSink & result, const char * & setSpec)
  {
    if (!setSpec)
      FAIL(NULL_POINTER);

    if (*setSpec++ != '[')
      FAIL(BAD_FORMAT_ARG);

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
          if (prevCh > next)
            FAIL(BAD_FORMAT_ARG);
          map.SetBits(prevCh, next - prevCh + 1);

          prevCh = next;  // Apparently [a-b-e] matches 'c', so..
          ++setSpec;

        }
      } else map.SetBit((u32) ch);
    }

    if (!complete)
      FAIL(BAD_FORMAT_ARG);

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

#if 0
  s32 ByteSource::Scanf(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    Vscanf(format, ap);
    va_end(ap);

  }
  s32 ByteSource::Vscanf(const char * format, va_list & ap) {
    MFM_API_ASSERT_NONNULL(format);

    u8 p;
    u32 result;

    bool alt;
    int matches = 0;
    while ((p = *format++)) {
      if (p != '%') {
        if (p == '\n') {            // '\n's _in_the_format_string_: We have to see EOF
          if (packetReadEOF(packet)) {
            ++matches;
            continue;               // ..but only end of format string now makes sense..
          } else return -matches;
        }
        if (!packetRead(packet,result,BYTE))
          return -matches;
        if ((char) result != p)
          return matches;
        ++matches;
        continue;
      }

      alt = false;
    again:
      u32 type = 0;
      switch (p = *format++) {
      case '#': alt = true; goto again;

      case 'c':
        if (!packetRead(packet,result,BYTE))
          return -matches;
        *getPtr1(ap) = result;
        ++matches;
        break;

      case 'h':
        if (!packetRead(packet,result,BESHORT))
          return -matches;
        *getPtr2(ap) = result;
        ++matches;
        break;

      case 'l': type = BELONG; goto store;
      case 'b': type = BIN; goto store;
      case 'o': type = OCT; goto store;
      case 'd': type = DEC; goto store;
      case 'x': type = HEX; goto store;
      case 't': type = B36; goto store;

      store:
        if (!packetRead(packet,result,type))
          return -matches;
        *getPtr4(ap) = result;
        ++matches;
        break;

      case 'F':
        if (!packetRead(packet,result,BYTE)) return -matches;
        if (!IS_FACE_CODE(result)) return matches;
        *getPtr1(ap) = FACE_NUMBER_FROM_CODE(result);
        ++matches;
        break;

#if 0
      case 'f': {
        double v =  va_arg(ap,double);
        facePrint(face,v);
        break;
      }

      case 's': {
        const char * s = va_arg(ap,const char *);
        if (!s) facePrint(face,"(null)");
        else facePrint(face,s);
        break;
      }
#endif

      case 'p': {
        u8 * subp = 0;
        if (!packetReadPacket(packet,subp)) return -matches;
        u8 ** ptr = va_arg(ap,u8**);
        API_ASSERT_NONNULL(ptr);
        *ptr = subp;
        ++matches;
        break;
      }

      case '%':
        if (!packetRead(packet,result,BYTE)) return -matches;
        if (result != '%') return matches;
        ++matches;
        break;

      case '\n':                  // "%\n" adds checkbyte then terminates packet!
        if (!packetReadCheckByte(packet)) return -matches;
        ++matches;
        break;

      default:                          // Either I don't know that code, or you're bogus.
        API_BUG(E_API_BAD_FORMAT_CODE); // Either way, I die.  You're welcome.
      }
    }
    return matches;
  }
#endif

}
