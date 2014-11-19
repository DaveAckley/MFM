#include "UUID.h"
#include "Fail.h"
#include "CharBufferByteSink.h"
#include <string.h>    /* For strcmp, strncpy */

namespace MFM {

  UUID::UUID(ByteSource & bs)
  {
    if (!ReadFrom(bs))
      FAIL(ILLEGAL_INPUT);
  }

  bool UUID::CompatibleLabel(const UUID & other) const
  {
    return m_label.Equals(other.m_label);
  }

  bool UUID::CompatibleConfigurationCode(const UUID & other) const
  {
    return m_configurationCode == other.m_configurationCode;
  }

  bool UUID::CompatibleAPIVersion(const UUID & other) const
  {
    return CompatibleLabel(other)
      && CompatibleConfigurationCode(other)
      && m_uuidVersion == other.m_uuidVersion
      && m_elementVersion == other.m_elementVersion;
  }

  bool UUID::CompatibleButStrictlyNewer(const UUID & other) const
  {
    return CompatibleAPIVersion(other) && CompareDateOnly(other) > 0;
  }

  bool UUID::Compatible(const UUID & other) const
  {
    return CompatibleAPIVersion(other) && CompareDateOnly(other) >= 0;
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
    bs.Print(GetLabel());
    bs.Printf("-%D%D%X%X%X",
              m_uuidVersion,
              m_elementVersion,
              m_configurationCode,
              m_hexDate,
              m_hexTime);
  }

  bool UUID::Read(ByteSource & bs) {

    // Read everything before changing anything..
    OString64 cbbs;
    u32 uuidversion = 0;
    u32 elementversion = 0;
    u32 configurationCode = 0;
    u32 hexdate = 0;
    u32 hextime = 0;

    if (!bs.ScanSet(cbbs, "[^-]"))
      return false;
    bs.Scanf("-");

    if (cbbs.HasOverflowed() || !LegalLabel(cbbs.GetZString()))
      return false;

    if (!bs.Scan(uuidversion, Format::LEX32)) return false;
    if (uuidversion != API_VERSION) return false;

    if (!bs.Scan(elementversion, Format::LEX32)) return false;
    if (!bs.Scan(configurationCode, Format::LXX32)) return false;
    if (!bs.Scan(hexdate, Format::LXX32)) return false;
    if (!bs.Scan(hextime, Format::LXX32)) return false;

    m_label.Reset();
    m_label.Print(cbbs.GetZString());
    m_label.GetZString();  // null terminate

    m_uuidVersion = uuidversion;
    m_elementVersion = elementversion;
    m_configurationCode = configurationCode;
    m_hexDate = hexdate;
    m_hexTime = hextime;
    return true;
  }

  bool UUID::operator==(const UUID & other) const
  {
    return
      m_uuidVersion == other.m_uuidVersion &&
      m_elementVersion == other.m_elementVersion &&
      m_hexDate == other.m_hexDate &&
      m_hexTime == other.m_hexTime &&
      strcmp(GetLabel(),other.GetLabel())==0;
  }

}
