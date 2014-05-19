#include "UUID.h"
#include "Fail.h"
#include "CharBufferByteSink.h"
#include <string.h>    /* For strcmp */

namespace MFM {
  UUID::UUID(const char * label, const u32 apiVersion, const u32 decDate, const u32 decTime)
    : m_label(label), m_apiVersion(apiVersion), m_hexDate(decDate), m_hexTime(decTime)
  {
    MFM_API_ASSERT_NONNULL(label);
  }

  bool UUID::CompatibleLabel(const UUID & other) const
  {
    return strcmp(m_label, other.m_label)==0;
  }

  bool UUID::CompatibleAPIVersion(const UUID & other) const
  {
    return CompatibleLabel(other) && m_apiVersion == other.m_apiVersion;
  }

  bool UUID::CompatibleButStrictlyNewer(const UUID & other) const
  {
    return CompatibleAPIVersion(other) && CompareDateOnly(other) > 0;
  }

  s32 UUID::CompareDateOnly(const UUID & other) const
  {
    if (m_hexDate > other.m_hexDate) return 1;
    if (m_hexDate < other.m_hexDate) return -1;
    if (m_hexTime > other.m_hexTime) return 1;
    if (m_hexTime < other.m_hexTime) return -1;
    return 0;
  }

  void UUID::Print(ByteSink & bs) const
  {
    bs.Print("[");

    u32 len = strlen(m_label);
    bs.Print(len, bs.LEXHD);
    bs.Print(m_label);
    bs.Printf("%D%08X%06X",
              m_apiVersion,
              m_hexDate,
              m_hexTime);

    bs.Print("]");
  }

  bool UUID::operator==(const UUID & other) const
  {
    return
      m_apiVersion == other.m_apiVersion &&
      m_hexDate == other.m_hexDate &&
      m_hexTime == other.m_hexTime &&
      strcmp(GetLabel(),other.GetLabel())==0;
  }

}

