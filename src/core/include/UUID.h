#ifndef UUID_H          /* -*- C++ -*- */
#define UUID_H

#include "itype.h"
#include "ByteSink.h"

#define MFM_UUID_FOR(label, apiVersion) UUID(label,apiVersion,MFM_BUILD_DATE,MFM_BUILD_TIME)

namespace MFM {

  struct UUID : public ByteSinkable {
    const char * const m_label;
    const u32 m_apiVersion;
    const u32 m_hexDate;
    const u32 m_hexTime;

    UUID(const char * label, const u32 apiVersion, const u32 hexDate, const u32 hexTime) ;

    const char * GetLabel() const { return m_label; }
    u32 GetVersion() const { return m_apiVersion; }
    u32 GetHexDate() const { return m_hexDate; }
    u32 GetHexTime() const { return m_hexTime; }

    bool CompatibleLabel(const UUID & other) const ;
    bool CompatibleAPIVersion(const UUID & other) const ;
    bool CompatibleButStrictlyNewer(const UUID & other) const ;

    void Print(ByteSink & bs) const ;

    void PrintTo(ByteSink & bs, s32 argument = 0) {
      Print(bs);
    }

    bool operator==(const UUID & other) const ;

    bool operator!=(const UUID & other) const {
      return !(*this == other);
    }

  private:
    s32 CompareDateOnly(const UUID & other) const ;
  };

}

#endif /* UUID_H */
