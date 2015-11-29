
#include "UlamClass.h"

namespace MFM {

  void UlamClass::addHex(ByteSink & bs, u64 val)
  {
    bs.Printf("/0x");
    bs.Print(val, Format::HEX);
  }

  bool UlamClassRegistry::RegisterUlamClass(UlamClass& uc)
  {
    if (IsRegisteredUlamClass(uc.GetMangledClassName()))
      return false;
    if (m_registeredUlamClassCount >= TABLE_SIZE)
      FAIL(OUT_OF_ROOM);
    m_registeredUlamClasses[m_registeredUlamClassCount++] = &uc;
    return true;
  }

  s32 UlamClassRegistry::GetUlamClassIndex(const char *mangledName) const
  {
    if (!mangledName) FAIL(NULL_POINTER);

    for (u32 i = 0; i < m_registeredUlamClassCount; ++i)
    {
      UlamClass * uc = m_registeredUlamClasses[i];
      if (!uc) FAIL(ILLEGAL_STATE);

      const char * mang = uc->GetMangledClassName();
      if (!strcmp(mang, mangledName))
        return (s32) i;
    }
    return -1;
  }

    bool UlamClassRegistry::IsRegisteredUlamClass(const char *mangledName) const
    {
      return GetUlamClassIndex(mangledName) >= 0;
    }

    const UlamClass* UlamClassRegistry::GetUlamClassByMangledName(const char *mangledName) const
    {
      s32 idx = GetUlamClassIndex(mangledName);
      if (idx < 0) return 0;
      return GetUlamClassByIndex((u32) idx);
    }

    const UlamClass* UlamClassRegistry::GetUlamClassByIndex(u32 index) const
    {
      if (index >= m_registeredUlamClassCount) FAIL(ILLEGAL_ARGUMENT);
      return m_registeredUlamClasses[index];
    }

} //MFM
