/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 150
#define MAX_ARG_LEN 64
#define MAX_ARGUMENTS 16

namespace MFM
{
  template<class GC>
  ExternalConfig<GC>::ExternalConfig(Grid<GC>& grid) :
    m_grid(grid), m_registeredFunctionCount(0)
  { }

  template<class GC>
  void ExternalConfig<GC>::SetByteSource(ByteSource & byteSource, const char * label)
  {
    m_in.SetByteSource(byteSource);
    m_in.SetLabel(label);
  }

  template<class GC>
  void ExternalConfig<GC>::SetErrorByteSink(ByteSink & errorsTo)
  {
    m_in.SetErrorByteSink(errorsTo);
  }

  template<class GC>
  bool ExternalConfig<GC>::Read()
  {

    m_grid.Clear();

    while (true) {

      m_in.SkipWhitespace();
      s32 ch = m_in.Read();

      if (ch < 0) {
        if (ch == EOF) break;
        return m_in.Msg(Logger::ERROR, "I/O error (%d)", -ch);
      }

      if (ch == '#') {
        // Line comment.  Skip to newline and try again
        m_in.SkipSet("[^\n]");
        continue;
      }

      m_in.Unread();
      OString64 cbbs;
      if (1 != m_in.Scanf("%[_a-zA-Z0-9]",&cbbs))
        return m_in.Msg(Logger::ERROR, "Expected function name");
      if (cbbs.HasOverflowed())
        return m_in.Msg(Logger::ERROR, "Function name too long '%@'", &cbbs);

      m_in.SkipWhitespace();
      if (1 != m_in.Scanf("("))
        return m_in.Msg(Logger::ERROR, "Expected open parenthesis");

      bool handled = false;
      for (u32 i = 0; i < m_registeredFunctionCount; ++i) {
        ConfigFunctionCall<GC> & fc = *m_registeredFunctions[i];
        if (cbbs.Equals(fc.m_functionName)) {
          if (!fc.Parse(*this)) return false;  // Error message already issued
          handled = true;
          break;
        }
      }

      if (!handled)
        return m_in.Msg(Logger::ERROR, "Unknown function '%s'",cbbs.GetZString());
    }
    return true;
  }

  template<class GC>
  void Write(ByteSink& byteSink)
  {
    FAIL(INCOMPLETE_CODE);
  }

  template<class GC>
  void ExternalConfig<GC>::RegisterFunction(ConfigFunctionCall<GC> & fc)
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

  template<class GC>
  bool ExternalConfig<GC>::RegisterElement(const UUID & uuid, OString16 & nick)
  {
    const UUID * puuid = &uuid;
    s32 type = U16StaticLoader::TypeFromUUID(uuid);
    if (type < 0) {
      m_in.Msg(Logger::WARNING, "Unknown element '%@', searching for compatible alternatives", &uuid);
      type = U16StaticLoader::TypeFromCompatibleUUID(uuid);
      if (type < 0)
        return m_in.Msg(Logger::WARNING, "No alternatives found for unknown/unregistered element '%@'", &uuid);
      puuid = U16StaticLoader::UUIDOfType((u32) type);
      if (!puuid) FAIL(ILLEGAL_STATE);
      m_in.Msg(Logger::WARNING, "Substituting '%@' for '%@'", puuid, &uuid);
    }

    for (u32 i = 0; i < m_registeredElementCount; ++i) {
      RegElt & re = m_registeredElements[i];
      if (puuid->Equals(re.m_uuid))
        return m_in.Msg(Logger::ERROR,"Element '%@' already registered as '%s'", &re.m_uuid, re.m_nick.GetZString());
      if (re.m_nick.Equals(nick))
        return m_in.Msg(Logger::ERROR,"Element nicknamed '%s' already registered for UUID '%@'",
                        nick.GetZString(), &re.m_uuid);
    }
    if (m_registeredElementCount >= sizeof(m_registeredElements)/sizeof(m_registeredElements[0]))
      return m_in.Msg(Logger::ERROR,"No room for more registered Elements");

    const Element<CC> * element = m_grid.LookupElement((u32) type);
    if (!element)
      return m_in.Msg(Logger::ERROR,"Registered UUID has no element?  Bug?  For %@/%04x", puuid, type);

    m_registeredElements[m_registeredElementCount].m_uuid = *puuid;
    m_registeredElements[m_registeredElementCount].m_nick = nick;
    m_registeredElements[m_registeredElementCount].m_element = element;
    ++m_registeredElementCount;
    return true;
  }

  template<class GC>
  const UUID * ExternalConfig<GC>::LookupElementNick(const OString16 & nick) const
  {
    for (u32 i = 0; i < m_registeredElementCount; ++i) {
      const RegElt & re = m_registeredElements[i];
      if (re.m_nick.Equals(nick))
        return &re.m_uuid;
    }
    return 0;
  }

  template<class GC>
  bool ExternalConfig<GC>::PlaceAtom(const UUID & uuid, s32 x, s32 y)
  {
    return m_in.Msg(Logger::ERROR,"Unimplemented");
  }

}
