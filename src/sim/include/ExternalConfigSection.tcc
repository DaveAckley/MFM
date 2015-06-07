/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "FileByteSink.h"  /* for STDERR */
#include "ExternalConfig.h"

#include <string.h>
#include <ctype.h>

namespace MFM
{
  template<class GC>
  ExternalConfigSection<GC>::ExternalConfigSection(ExternalConfig<GC> & ec)
    : m_ec(ec)
    , m_registeredFunctionCount(0)
  {
  }

  template<class GC>
  LineCountingByteSource & ExternalConfigSection<GC>::GetByteSource()
  {
    return m_ec.GetByteSource();
  }

  template<class GC>
  bool ExternalConfigSection<GC>::ReadSection()
  {
    LineCountingByteSource & lcbs = m_ec.GetByteSource();

    if (!ReadInit())
      return false;

    OString64 cbbs;
    while (true) {

      lcbs.SkipWhitespace();
      s32 ch = lcbs.Read();

      if (ch < 0) {
        if (ch == EOF) break;
        return lcbs.Msg(Logger::ERROR, "I/O error (%d)", -ch);
      }

      if (ch == '#') {
        // Line comment.  Skip to newline and try again
        lcbs.SkipSet("[^\n]");
        continue;
      }

      if (ch == '[') {
        // End of current section or error
        const char * secname = GetSectionName();
        cbbs.Reset();
        if (3 != lcbs.Scanf("/%[_a-zA-Z0-9]]",&cbbs) ||
            !cbbs.Equals(secname))
          return lcbs.Msg(Logger::ERROR, "Expected [/%s]", secname);

        return ReadFinalize();
      }

      lcbs.Unread();
      cbbs.Reset();
      if (1 != lcbs.Scanf("%[_a-zA-Z0-9]",&cbbs))
        return lcbs.Msg(Logger::ERROR, "Expected function name");
      if (cbbs.HasOverflowed())
        return lcbs.Msg(Logger::ERROR, "Function name too long '%@'", &cbbs);

      lcbs.SkipWhitespace();
      if (1 != lcbs.Scanf("("))
        return lcbs.Msg(Logger::ERROR, "Expected open parenthesis after '%@'", &cbbs);

      bool handled = false;
      u32 functionErrors = 0;
      for (u32 i = 0; i < m_registeredFunctionCount; ++i) {
        ConfigFunctionCall<GC> & fc = *m_registeredFunctions[i];
        if (cbbs.Equals(fc.m_functionName)) {
          if (!fc.Parse()) {
            if (this->ContinueOnErrors())
            {
              ++functionErrors;
              lcbs.Msg(Logger::ERROR, "[%s] error (#%d) during '%s' load, trying to continue",
                       GetSectionName(),
                       functionErrors,
                       fc.m_functionName);
              lcbs.SkipSet("[^\n]");
              handled = true;
              break;
            }
            lcbs.Msg(Logger::ERROR, "[%s] fatal error during '%s' load, aborting",
                     GetSectionName(),
                     fc.m_functionName);
            return false;
          }
          handled = true;
        }
      }

      if (!handled)
        return lcbs.Msg(Logger::ERROR, "Function '%s' unknown in section [%s]",
                        cbbs.GetZString(),
                        GetSectionName());
    }
    return true;
  }

  template<class GC>
  void ExternalConfigSection<GC>::Write(ByteSink& byteSink)
  {
    byteSink.Printf("\n[%s]\n", GetSectionName());

    this->WriteSection(byteSink);

    byteSink.Printf("[/%s]\n", GetSectionName());
  }

  template<class GC>
  void ExternalConfigSection<GC>::RegisterFunction(ConfigFunctionCall<GC> & fc)
  {
    for (u32 i = 0; i < m_registeredFunctionCount; ++i) {
      ConfigFunctionCall<GC> & rfc = *m_registeredFunctions[i];
      if (!strcmp(fc.m_functionName, rfc.m_functionName))
        FAIL(DUPLICATE_ENTRY);
    }
    if (m_registeredFunctionCount >= sizeof(m_registeredFunctions)/sizeof(m_registeredFunctions[0]))
      FAIL(OUT_OF_ROOM);

    m_registeredFunctions[m_registeredFunctionCount++] = &fc;
  }

}
