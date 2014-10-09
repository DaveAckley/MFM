#include "ByteSink.h"
#include "ByteSerializable.h"
#include <string.h>   /* For strlen */

namespace MFM {

  DiscardBytes DevNull;

  void ByteSink::Print(ByteSerializable & byteSerializable, s32 argument)
  {
    if (byteSerializable.PrintTo(*this, argument) == ByteSerializable::UNSUPPORTED)
      Print("(unsupported)");
  }

  void ByteSink::Println(ByteSerializable & byteSerializable, s32 argument) {
    Print(byteSerializable, argument);
    Println();
  }



/**
   Print the contents of a null-terminated string to the ByteSink.
   Compare to ::Print(const u8 * str, u32 len).

  \param str A pointer to the null-terminate string to print.

  \fails #NULL_POINTER if \a str is null.

 */
  void ByteSink::Print(const char * str, s32 fieldWidth, u8 padChar)
  {
    if (!str)
      FAIL(NULL_POINTER);

    s32 len = strlen(str);

    while (fieldWidth > len) {
      WriteByte(padChar);
      --fieldWidth;
    }

    u8 ch;
    while ((ch = *str++))
      WriteByte(ch);
  }

  /**
     Print \a str, an array of bytes of length \a len, to the
     ByteSink.  Compare to ::Print(const char * str).

     \param str A pointer to the byte array to print.  Must be of
     length (at least) \a len; does not need to be null-terminated.

     \param len The number of bytes in \a str to output.  May be zero.

     \fails NULL_POINTER if \a str is null.
  */

  void ByteSink::Print(const u8 * str, u32 len, s32 fieldWidth, u8 padChar)
  {
    if (!str)
      FAIL(NULL_POINTER);

    while (fieldWidth > (s32) len) {
      WriteByte(padChar);
      --fieldWidth;
    }
    while (len--)
      WriteByte(*str++);

  }

  void ByteSink::Print(s32 decimal, s32 fieldWidth, u8 padChar)
  {
    if (decimal < 0) {
      if (decimal-1 > 0) {
        // Argh.  Printing this one value right is a huge pain.
        if (padChar == '0' && fieldWidth > 11) {
          Print("-");
          for (s32 i = 11; i < fieldWidth; ++i)
            Print("0");
          Print("2147483648");
        } else
          Print("-2147483648", fieldWidth, padChar);
        return;
      }
      decimal = -decimal;

      u32 len = CountDigits((u32) decimal, 10) + 1;

      if (padChar == '0') WriteByte('-');
      while (fieldWidth > (s32) len) {
        WriteByte(padChar);
        --fieldWidth;
      }
      if (padChar != '0') WriteByte('-');
      fieldWidth = 0;
    }
    Print((u32) decimal, fieldWidth, padChar);
  }

  void ByteSink::Print(u64 num, s32 fieldWidth, u8 padChar)
  {
    PrintInBase(num, Format::DEC, fieldWidth, padChar);
  }

  void ByteSink::Print(s64 decimal, s32 fieldWidth, u8 padChar)
  {
    if (decimal < 0) {
      if (decimal-1 > 0) {
        Print("-9223372036854775808");
        return;
      }
      decimal = -decimal;
      u32 len = CountDigits((u64) decimal, 10) + 1;
      while (fieldWidth > (s32) len) {
        WriteByte(padChar);
        --fieldWidth;
      }
      WriteByte('-');
    }

    Print((u64) decimal, fieldWidth, padChar);
  }

  void ByteSink::Print(u32 decimal, s32 fieldWidth, u8 padChar)
  {
    Print(decimal, Format::DEC, fieldWidth, padChar);
  }

  void ByteSink::PrintLexDigits(u32 digits) {
    if (digits > 8) {
      Print('9', Format::BYTE);
      Print(digits, Format::LEX32);
    } else {
      Print(digits+'0', Format::BYTE);
    }
  }

/**
   Print \a num, in a format selected by \a code, to the current xmit packet of \a face.  The possible
   format codes are:

   - #DEC: Print one to ten bytes containing the base-10 representation of the 32 bits of \a num,
      using the ASCII characters '0' through '9'.  See the '%d' code in #packetScanf(u8 * packet,
      const char * format,...)  for a way to read in a value that was printed with code \c DEC, and
      in #facePrintf(u8 face, const char * format,...)  for an alternate way to print in this
      representation.
   - #HEX: Print one to eight bytes containing the base-16 representation of the 32 bits of \a num,
      using the ASCII characters '0' through '9' to represent 0 through 9, and 'A' through 'F' to
      represent 10 through 15.  See the '%x' code in #packetScanf(u8 * packet, const char *
      format,...) for a way to read in a value that was printed with code \c HEX, and in
      #facePrintf(u8 face, const char * format,...)  for an alternate way to print in this
      representation.
   - #OCT: Print one to eleven bytes containing the base-8 representation of the 32 bits of \a num,
      using the ASCII characters '0' through '7' to represent 0 through 7.  See the '%x' code in
      #packetScanf(u8 * packet, const char * format,...)  for a way to read in a value that was
      printed with code \c OCT, and in #facePrintf(u8 face, const char * format,...)  for an
      alternate way to print in this representation.
   - #BIN: Print one to 32 bytes containing the base-2 representation of the 32 bits of \a num,
      using the ASCII characters '0' and '1' to represent 0 and 1.  See the '%b' code in
      #packetScanf(u8 * packet, const char * format,...)  for a way to read in a value that was
      printed with code \c BIN, and in #facePrintf(u8 face, const char * format,...)  for an
      alternate way to print in this representation.
   - #B36: Print one to six bytes containing the base-36 representation of the 32 bits of \a num,
      using the ASCII characters '0' through '9' to represent 0 through 9, and 'A' through 'Z' to
      represent 10 through 35.  See the '%t' code in #packetScanf(u8 * packet, const char *
      format,...)  for a way to read in a value that was printed with code \c B36, and in
      #facePrintf(u8 face, const char * format,...)  for an alternate way to print in this
      representation.
   - #BEU32: Print exactly four bytes containing the 32 bits of \a num, with the four bytes in 'big
      endian order', also known as 'network order'.  See the '%l' code in #packetScanf(u8 * packet,
      const char * format,...) for a way to read in a value that was printed with code \c BEU32,
      and in #facePrintf(u8 face, const char * format,...)  for an alternate way to print in this
      representation.\n
      \c BEU32 is called a 'binary' code, but it must not be confused with #BIN.  With \c BEU32,
      each of the four output bytes may each contain any possible bit pattern, whereas with code
      #BIN each output byte is either the ASCII code for '0' or the ASCII code for '1'.
   - #BEU16: Print exactly two bytes containing the low-order 16 bits of \a num, with the two
      bytes in 'big endian order', also known as 'network order'.  See the '%h' code in
      #packetScanf(u8 * packet, const char * format,...) for a way to read in a value that was
      printed with code \c BEU16, and in #facePrintf(u8 face, const char * format,...)  for an
      alternate way to print in this representation.\n
      \c BEU16 is called a 'binary' code, but it must not be confused with #BIN.  With \c BEU16,
      each of the two output bytes may each contain any possible bit pattern, whereas with code #BIN
      each output byte is either the ASCII code for '0' or the ASCII code for '1'.
   - #BYTE: Print exactly one byte containing the low-order 8 bits of \a num.  See the '%c' code in
      #packetScanf(u8 * packet, const char * format,...) for a way to read in a value that was
      printed with code \c BYTE, and in #facePrintf(u8 face, const char * format,...)  for an
      alternate way to print in this representation.\n
      \c BYTE is called a 'binary' code, but it must not be confused with #BIN.  With \c BYTE, the
      output byte may each contain any possible bit pattern, whereas with code #BIN each output byte
      is either the ASCII code for '0' or the ASCII code for '1'.

   In addition to those codes, it is also possible to print in other bases between 2 and 36, if
   there is call to do so.  For example, using a \a code value of 13 will select printing in base
   13, using the ASCII characters '0' through '9' and 'A' through 'C'.

  \param num A 32 bit number to be printed in a particular format.

  \param code What format to use (see above).

  \fails BAD_FORMAT_ARG If \a code is an illegal code.  The legal codes are #BEU32, #BEU16 or
  #BYTE, or any value from 2 to 36 (which includes #DEC, #HEX, #OCT, #BIN, and #B36).

XXX
  Sample all-ASCII printable packet generation:
  \usage
  \code
    void myHandler(u8 *) {
      facePrint(SOUTH,"L 1234 in:");
      facePrint(SOUTH," base 10="); facePrint(SOUTH, 1234, DEC); // Prints 1234
      facePrint(SOUTH," base 16="); facePrint(SOUTH, 1234, HEX); // Prints 4D2
      facePrint(SOUTH," base 8=");  facePrint(SOUTH, 1234, OCT); // Prints 2322
      facePrint(SOUTH," base 2=");  facePrint(SOUTH, 1234, BIN); // Prints 10011010010
      facePrint(SOUTH," base 36="); facePrint(SOUTH, 1234, B36); // Prints YA
      facePrintln(SOUTH);
    }
  \endcode

XXX
  Sample binary packet generation:
  \usage
  \code
    void setup() { /\* nothing to do *\/ }
    void loop() {                                  // Print a packet containing exactly eight bytes:
      facePrint(ALL_FACES,"t");                    //  1 byte containing an ASCII 't'
      facePrint(ALL_FACES,millis(),BEU32);        // +4 bytes of board uptime in big endian
      facePrint(ALL_FACES,random(10000),BEU16);  // +2 bytes of a random number 0..9999 in big endian
      facePrintln();                               // +1 byte containing an ASCII newline '\n'
      delay(1000);                                 // Wait a second, then do it again.
    }
  \endcode
 */
  void ByteSink::Print(u32 num, Format::Type code, s32 fieldWidth, u8 padChar)
  {

    switch (code) {
    case Format::LEX64:
    case Format::BEU64:
      FAIL(ILLEGAL_ARGUMENT);

    case Format::LEXHD:  // padding makes no sense for a lex header
      PrintLexDigits(num);
      break;

    case Format::LEX32:
    case Format::LXX32:
      {
        u32 base = 10;
        if (code==Format::LXX32)
          base = 16;

        u32 digits = CountDigits(num, base);
        if (fieldWidth > (s32) digits)
          digits = fieldWidth;

        PrintLexDigits(digits);
        Print(num, (Format::Type) base, fieldWidth, padChar);
        break;
      }

    case Format::BEU32:  // padding makes no sense for binary
      WriteByte((num>>24)&0xff);
      WriteByte((num>>16)&0xff);
      /* FALL THROUGH */

    case Format::BEU16:
      WriteByte((num>>8)&0xff);
      /* FALL THROUGH */

    case Format::BYTE:
      WriteByte((num>>0)&0xff);
      break;

    default:
      PrintInBase(num, (u32) code, fieldWidth, padChar);
    }
  }

  void ByteSink::Print(u64 num, Format::Type code, s32 fieldWidth, u8 padChar)
  {

    switch (code) {

    case Format::LXX64:
    case Format::LEX64: {
      u32 base = 10;
      if (code==Format::LXX64)
        base = 16;

      u32 digits = CountDigits(num, base);
      if (fieldWidth > (s32) digits)
        digits = fieldWidth;

      PrintLexDigits(digits);
      PrintInBase(num, (Format::Type) base, fieldWidth, padChar);
      break;
    }

    case Format::BEU64:
    case Format::BEU32:
      WriteByte((num>>(24+32))&0xff);
      WriteByte((num>>(16+32))&0xff);
      WriteByte((num>>( 8+32))&0xff);
      WriteByte((num>>( 0+32))&0xff);

      WriteByte((num>>(24+ 0))&0xff);
      WriteByte((num>>(16+ 0))&0xff);
      WriteByte((num>>( 8+ 0))&0xff);
      WriteByte((num>>( 0+ 0))&0xff);
      break;

    default:
      PrintInBase(num, code, fieldWidth, padChar);
      break;
    }
  }

  /**
     The same as #Print(u32 num, Format code), \e except that if \a
     num is negative \e and \a code is \c DEC, then prints \a num as a
     negative, with a leading '-'.

     \param num A 32 bit number to be printed in a particular format.

     \param code What format to use

     \fails BAD_FORMAT_ARG If \a code is an illegal code.  The legal
     codes are #BEU32, #BEU16 or #BYTE, or any value from 2 to 36
     (which includes #DEC, #HEX, #OCT, #BIN, and #B36).

     \usage
     XXX
     \code
     ...
     int signedNum = -1;
     u32 unsignedNum = signedNum;
     facePrint(WEST,signedNum,DEC);               // Prints "-1": Signed num with code DEC
     facePrint(WEST,unsignedNum,DEC);             // Prints "4294967295": Unsigned num with code DEC
     facePrint(WEST,signedNum,HEX);               // Prints "FFFFFFFF": Non-DEC codes are always
     facePrint(WEST,unsignedNum,HEX);             // Prints "FFFFFFFF": printed as unsigned
     ...
     \endcode
  */
  void ByteSink::Print(s32 num, Format::Type code, s32 fieldWidth, u8 padChar) {
    if (code == Format::DEC) {
      Print(num, fieldWidth, padChar);
      return;
    }
    Print((u32) num, code, fieldWidth, padChar);
  }

  void ByteSink::Print(s64 num, Format::Type code, s32 fieldWidth, u8 padChar) {
    if (code == Format::DEC) {
      Print(num, fieldWidth, padChar);
      return;
    }
    Print((u64) num, code, fieldWidth, padChar);
  }

  void ByteSink::Printf(const char * format, ...)
  {
    va_list ap;
    va_start(ap, format);
    Vprintf(format, ap);
    va_end(ap);
  }

  void ByteSink::Vprintf(const char * format, va_list & ap)
  {
    MFM_API_ASSERT_NONNULL(format);

    u8 p;
    Format::Type type;
    bool alt;
    s32 fieldWidth;
    u8 padChar;
    while ((p = *format++)) {
      if (p != '%') {
        if (p == '\n')          // '\n's _in_the_format_string_ are
          Println();            // treated as packet delimiters!
        else
          Print(p,Format::BYTE);
        continue;
      }

    alt = false;
    fieldWidth = -1;
    padChar = ' ';
  again:
    switch (p = *format++) {
    case '#': alt = true; goto again;

    case '0':
      if (fieldWidth < 0) {
        padChar = '0';
        fieldWidth = 0;
      } else fieldWidth *= 10;
      goto again;

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      if (fieldWidth < 0) fieldWidth = 0;
      fieldWidth = fieldWidth * 10 + (p - '0');
      goto again;

    case 'c':
      {
        u32 ch = va_arg(ap,int);
        if (!alt || IsPrint(ch)) Print(ch,Format::BYTE);
        else {
          Print('[',Format::BYTE);
          if (ch < 0x10)
            Print('0', Format::BYTE);
          Print(ch, Format::HEX);
          Print(']', Format::BYTE);
        }
      }
      break;

    case '@':
      {
        s32 argument = 0;
        if (alt) argument = va_arg(ap,s32);
        ByteSerializable * bs = va_arg(ap,ByteSerializable*);
        if (!bs) Print("(null)");
        else Print(*bs, argument);
      }
      break;

    case 'q':
      Print(va_arg(ap,u64), Format::BEU64);
      break;
    case 'H': type = Format::LEXHD; goto print;
    case 'h': type = Format::BEU16; goto print;
    case 'l': type = Format::BEU32; goto print;

    case 'b': type = Format::BIN; goto printbase;
    case 'o': type = Format::OCT; goto printbase;
    case 'd':
      if (alt) {
        type = Format::DEC;
        goto printbase;
      } else {
        Print(va_arg(ap,s32), fieldWidth, padChar);
        break;
      }
    case 'x': type = Format::HEX; goto printbase;
    case 't': type = Format::B36; goto printbase;

    case 'D': type = Format::LEX32; goto print;
    case 'X': type = Format::LXX32; goto print;

    printbase:
      PrintInBase(va_arg(ap,u32), type, fieldWidth, padChar);
      break;
    print:
      Print(va_arg(ap,u32),type, fieldWidth, padChar);
      break;

    case 'f': {
      FAIL(INCOMPLETE_CODE);
      /*
      double v =  va_arg(ap,double);
      ByteSink::Print(face,v);
      break;
      */
    }

    case 's': {
      const char * s = va_arg(ap,const char *);
      if (!s) Print("(null)", fieldWidth, padChar);
      else Print(s, fieldWidth, padChar);
      break;
    }

    case 'p': {
      const void * p = va_arg(ap,void *);
      if (!p) Print("(nullp)");
      else {
        Print("0x");
        Print((uptr) p, Format::HEX);
      }
      break;
    }

    case '%':
      Print(p, Format::BYTE);
      break;

    default:                    // Either I don't know that code, or you're bogus.
      FAIL(BAD_FORMAT_ARG);     // Either way, I die.  You're welcome.
    }
  }
}

}
