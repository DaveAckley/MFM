/* -*- C++ -*- */

#include "UlamClass.h"

namespace MFM {

  template <class EC>
  s32 UlamClassRegistry<EC>::GetUlamClassIndex(const char *mangledName) const
  {
    if (!mangledName) FAIL(NULL_POINTER);

    for (u32 i = 0; i < m_registeredUlamClassCount; ++i)
    {
      UlamClass<EC> * uc = m_registeredUlamClasses[i];
      if (!uc) FAIL(ILLEGAL_STATE);

      const char * mang = uc->GetMangledClassName();
      if (!strcmp(mang, mangledName))
        return (s32) i;
    }
    return -1;
  }

  template <class EC>
  bool UlamClassRegistry<EC>::RegisterUlamClass(UlamClass<EC>& uc)
  {
    if (IsRegisteredUlamClass(uc.GetMangledClassName()))
      return false;
    if (m_registeredUlamClassCount >= TABLE_SIZE)
      FAIL(OUT_OF_ROOM);
    m_registeredUlamClasses[m_registeredUlamClassCount++] = &uc;
    return true;
  }

  template <class EC>
  bool UlamClassRegistry<EC>::IsRegisteredUlamClass(const char *mangledName) const
  {
    return GetUlamClassIndex(mangledName) >= 0;
  }

  template <class EC>
  const UlamClass<EC>* UlamClassRegistry<EC>::GetUlamClassByMangledName(const char *mangledName) const
  {
    s32 idx = GetUlamClassIndex(mangledName);
    if (idx < 0) return 0;
    return GetUlamClassByIndex((u32) idx);
  }

  template <class EC>
  const UlamClass<EC>* UlamClassRegistry<EC>::GetUlamClassByIndex(u32 index) const
  {
    if (index >= m_registeredUlamClassCount) FAIL(ILLEGAL_ARGUMENT);
    return m_registeredUlamClasses[index];
  }

} //MFM
