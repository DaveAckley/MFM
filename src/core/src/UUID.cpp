#include "UUID.h"
#include "Fail.h"
#include "CharBufferByteSink.h"
#include <string.h>    /* For strcmp, strncpy */

namespace MFM {
  UUID::UUID(const char * label, const u32 apiVersion, const u32 decDate, const u32 decTime)
    : m_apiVersion(apiVersion), m_hexDate(decDate), m_hexTime(decTime)
  {
    if (!label)
      FAIL(NULL_POINTER);
    strncpy(m_label, label, MAX_LABEL_LENGTH + 1);
  }

  UUID::UUID(ByteSource & bs)
  {
    if (!ReadFrom(bs))
      FAIL(ILLEGAL_INPUT);
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
    u32 len = strlen(m_label);
    bs.Print(len, Format::LEXHD);
    bs.Print(m_label);
    bs.Printf("%D%08X%06X",
              m_apiVersion,
              m_hexDate,
              m_hexTime);
  }

  bool UUID::Read(ByteSource & bs) {

    // Read everything before changing anything..
    CharBufferByteSink<UUID::MAX_LABEL_LENGTH> cbbs;
    u32 apiversion = 0;
    u32 hexdate = 0;
    u32 hextime = 0;

    u32 hdlen = 0;
    if (!bs.Scan(hdlen, Format::LEXHD)) return false;
    if (!bs.Scan(cbbs, hdlen)) return false;
    if (!bs.Scan(apiversion, Format::LEX32)) return false;
    if (!bs.Scan(hexdate, Format::LXX32)) return false;
    if (!bs.Scan(hextime, Format::LXX32)) return false;

    strncpy(m_label, cbbs.GetZString(), MAX_LABEL_LENGTH + 1);
    m_apiVersion = apiversion;
    m_hexDate = hexdate;
    m_hexTime = hextime;
    return true;
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

