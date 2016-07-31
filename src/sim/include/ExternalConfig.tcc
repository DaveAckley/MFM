/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AtomSerializer.h"
#include <string.h>
#include <ctype.h>

namespace MFM
{
  template<class GC>
  ExternalConfig<GC>::ExternalConfig(AbstractDriver<GC>& driver)
    : m_driver(driver)
    , m_registeredSectionCount(0)
  {
    m_in.SetErrorByteSink(STDERR);
  }

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
    // Reset each section
    for (u32 i = 0; i < m_registeredSectionCount; ++i) {
      ExternalConfigSection<GC> * ecs = m_registeredSections[i];
      ecs->Reset();
    }

    /* Only thing not in a section (and before all sections) is
       the mfs version. */

    m_in.SkipWhitespace();
    u32 version;
    if (6 != m_in.Scanf("MFS/%d\n",&version))
      return m_in.Msg(Logger::ERROR, "Expected MFS version header");

    if (version > MFS_VERSION)
      return m_in.Msg(Logger::ERROR, "File version MFS/%u newer than us (MFS/%u)",
                      version, MFS_VERSION);

    if (version < MFS_VERSION)
      m_in.Msg(Logger::WARNING, "File version MFS/% older than us (MFS/%u)",
               version, MFS_VERSION);

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

      OString32 sectionName;
      if (3 != m_in.Scanf("[%[_a-zA-Z0-9]]",&sectionName))
        return m_in.Msg(Logger::ERROR, "Expected [section] name");

      if (sectionName.HasOverflowed())
        return m_in.Msg(Logger::ERROR, "Section name too long '[%@]'", &sectionName);

      bool found = false;

      for (u32 i = 0; i < m_registeredSectionCount; ++i) {
        ExternalConfigSection<GC> * ecs = m_registeredSections[i];
        if (sectionName.Equals(ecs->GetSectionName())) {
          found = true;
          if (!ecs->ReadSection()) return false;  // Error message already issued
          break;
        }
      }

      if (!found)
      {
        // Try to skip unknown section
        while (true)
        {
          ch = m_in.Read();
          if (ch == EOF)
            return m_in.Msg(Logger::ERROR, "EOF while searching for '[/%s]'", sectionName.GetZString());

          if (ch != '\n') continue;

          m_in.SkipWhitespace();

          OString64 cbbs;
          if (1 != m_in.Scanf("[/%[_a-zA-Z0-9]]",&cbbs))
            continue;

          if (!cbbs.Equals(sectionName))
            return m_in.Msg(Logger::ERROR, "Cannot close '[%s]' with '[/%s]'",
                            sectionName.GetZString(),
                            cbbs.GetZString());

          break; // matching section close found
        }
        m_in.Msg(Logger::WARNING, "Skipped unknown section '[%s]'",
                 sectionName.GetZString());
        continue; // back to section loop
      }
    }
    return true;
  }

  template<class GC>
  void ExternalConfig<GC>::Write(ByteSink& byteSink)
  {
    /* First, identify mfs version. */
    byteSink.Printf("MFS/%u\n", MFS_VERSION);

    /* Then each section goes, in registration order */
    for (u32 i = 0; i < m_registeredSectionCount; ++i) {
      ExternalConfigSection<GC> * ecs = m_registeredSections[i];
      byteSink.Printf("\n[%s]\n",ecs->GetSectionName());
      ecs->WriteSection(byteSink);
      byteSink.Printf("[/%s]\n",ecs->GetSectionName());
    }
  }

  template<class GC>
  void ExternalConfig<GC>::RegisterSection(ExternalConfigSection<GC>& section)
  {
    for (u32 i = 0; i < m_registeredSectionCount; ++i) {
      ExternalConfigSection<GC> & ecs = *m_registeredSections[i];
      if (!strcmp(section.GetSectionName(), ecs.GetSectionName()))
        FAIL(DUPLICATE_ENTRY);
    }
    if (m_registeredSectionCount >= sizeof(m_registeredSections)/sizeof(m_registeredSections[0]))
      FAIL(OUT_OF_ROOM);

    m_registeredSections[m_registeredSectionCount++] = &section;
  }

}
