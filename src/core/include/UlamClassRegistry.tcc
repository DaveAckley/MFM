/* -*- C++ -*- */

#include "UlamClass.h"

namespace MFM {

  template <class EC>
  s32 UlamClassRegistry<EC>::GetUlamClassIndex(const char *mangledName) const
  {
    if (!mangledName) FAIL(NULL_POINTER);

    // HACK: If mangledName is an array type, we need to get the
    // mangled name representing the underlying scalar type, for
    // lookup purposes.
    UlamTypeInfo uti;
    OString512 scalarName;
    if (!uti.InitFrom(mangledName))
      FAIL(ILLEGAL_ARGUMENT);

    if (uti.GetArrayLength() > 0) {

      uti.MakeScalar();                // Stomp out the array length
      uti.PrintMangled(scalarName);    // Convert back to mangled name
      mangledName = scalarName.GetZString(); // Update pointer
    }

    for (u32 i = 0; i < m_registeredUlamClassCount; ++i)
    {
      UlamClass<EC> * uc = m_registeredUlamClasses[i];
      if (!uc) continue; //contiguous after all classes register

      const char * mang = uc->GetMangledClassName();
      if (!strcmp(mang, mangledName))
        return (s32) i;
    }
    return -1;
  }

  template <class EC>
  bool UlamClassRegistry<EC>::RegisterUlamClass(UlamClass<EC>& uc)
  {
    //if (IsRegisteredUlamClass(uc.GetMangledClassName()))
    //  return false;

    //culam assigns the UlamClass Registry Number to each class
    //(after c&l, before gencode) (ulam-4)
    u32 myregnum = uc.GetRegistrationNumber();
    if (myregnum >= TABLE_SIZE)
      FAIL(OUT_OF_ROOM);
    if(m_registeredUlamClasses[myregnum] != NULL)
      {
	if(m_registeredUlamClasses[myregnum] == &uc)
	  return false; //is already registered, return false
	else
	  FAIL(DUPLICATE_ENTRY);
      }

    m_registeredUlamClasses[myregnum] = &uc;
    if(myregnum >= m_registeredUlamClassCount)
      m_registeredUlamClassCount = myregnum + 1; //max + 1

    return true;
  }

  template <class EC>
  s32 UlamClassRegistry<EC>::RegisterUlamElementEmpty(UlamClass<EC>& ue)
  {
    if (strcmp(ue.GetMangledClassName(), "Ue_10105Empty10"))
      return 0;
    if (!m_ulamElementEmpty)
    {
      m_ulamElementEmpty = &ue;
      return 1;
    }
    return -1;
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
    if(!m_registeredUlamClasses[index]) FAIL(ILLEGAL_STATE);
    return m_registeredUlamClasses[index];
  }

  template <class EC>
  const UlamClass<EC>* UlamClassRegistry<EC>::GetUlamClassOrNullByIndex(u32 index) const
  {
    if (index >= m_registeredUlamClassCount) return 0;
    return m_registeredUlamClasses[index];
  }

} //MFM
