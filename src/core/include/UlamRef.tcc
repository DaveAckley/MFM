/* -*- C++ -*- */
#include "Fail.h"
#include "BitVector.h"
#include "UlamElement.h"

namespace MFM {

  template <class EC>
  UlamRef<EC>::UlamRef(u32 pos, u32 len, BitStorage<EC>& stg, const UlamClass<EC> * effself,
                       const UsageType usage, const UlamContext<EC> & uc)
    : m_uc(uc)
    , m_effSelf(effself)
    , m_stg(stg)
    , m_pos(pos)
    , m_len(len)
    , m_usage(usage)
  {
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    MFM_API_ASSERT_ARG(m_usage != PRIMITIVE || m_effSelf == 0); // Primitive usage has no effself
    MFM_API_ASSERT_ARG(m_usage != ARRAY || m_effSelf == 0); // Array usage has no effself
    MFM_API_ASSERT_ARG(m_usage != CLASSIC || m_effSelf != 0); // Classic usage has effself

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef & existing, s32 pos, u32 len, const UlamClass<EC> * effself, const UsageType usage)
    : m_uc(existing.m_uc)
    , m_effSelf(effself)
    , m_stg(existing.m_stg)
    , m_len(len)
    , m_usage(usage)
  {
    s32 newpos = pos + (s32) existing.GetPos(); //e.g. pos -25 to start of atom of element ref
    MFM_API_ASSERT_ARG(newpos >= 0); //non-negative
    m_pos = (u32) newpos; //save as unsigned

    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    MFM_API_ASSERT_ARG(existing.m_usage != PRIMITIVE || m_usage == existing.m_usage);  // derived from PRIMITIVE can't change usage type
    MFM_API_ASSERT_ARG(m_usage != ARRAY || m_effSelf == 0); // Array usage has no effself
    MFM_API_ASSERT_ARG(m_usage != CLASSIC || m_effSelf != 0); // Classic usage has effself

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef & existing, u32 len)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_pos(existing.m_pos)
    , m_len(len)
    , m_usage(existing.m_usage)
  {
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }
  }


  template <class EC>
  void UlamRef<EC>::UpdateEffectiveSelf()
  {
    m_effSelf = LookupElementTypeFromAtom();
  }

  template <class EC>
  void UlamRef<EC>::CheckEffectiveSelf() const
  {
    if (m_usage == ATOMIC || m_usage == ELEMENTAL)
    {
      const UlamClass<EC> * eltptr = LookupElementTypeFromAtom();
      MFM_API_ASSERT(eltptr->internalCMethodImplementingIs(m_effSelf), STALE_ATOM_REF);
    }
  }

  template <class EC>
  const UlamClass<EC>* UlamRef<EC>::LookupElementTypeFromAtom() const
  {
    MFM_API_ASSERT_STATE(m_usage == ATOMIC || m_usage == ELEMENTAL);
    T a = ReadAtom();
    MFM_API_ASSERT(a.IsSane(),INCONSISTENT_ATOM);
    u32 etype = a.GetType();
    const UlamClass<EC> * eltptr = m_uc.LookupElementTypeFromContext(etype);
    MFM_API_ASSERT_STATE(eltptr);
    return eltptr;
  }

  template <class EC>
  u32 UlamRef<EC>::GetType() const
  {
    const UlamClass<EC> * effSelf = GetEffectiveSelf();
    MFM_API_ASSERT_ARG(effSelf);
    const UlamElement<EC> * eltptr = effSelf->AsUlamElement();
    if(!eltptr) return T::ATOM_UNDEFINED_TYPE; //quark
    return eltptr->GetType();
  } //GetType

  template <class EC>
  typename EC::ATOM_CONFIG::ATOM_TYPE UlamRef<EC>::CreateAtom() const
  {
    const UlamClass<EC> * effSelf = GetEffectiveSelf();
    MFM_API_ASSERT_ARG(effSelf);
    const UlamElement<EC> * eltptr = effSelf->AsUlamElement();
    if(!eltptr) FAIL(ILLEGAL_ARGUMENT);
    u32 len = eltptr->GetClassLength();
    AtomBitStorage<EC> atmp(eltptr->GetDefaultAtom());
    atmp.WriteBig(0u + T::ATOM_FIRST_STATE_BIT, len, m_stg.ReadBig(GetPos(), len));
    return atmp.ReadAtom();
  }

  template <class EC>
  void UlamRef<EC>::Print(const UlamClassRegistry<EC>&uc, ByteSink& bs, u32 printFlags) const
  {
    if (!m_effSelf)
    {
      bs.Printf("UlamRef[pos=%d,len=%d,NULL]", m_pos, m_len);
      return;
    }

    const UlamElement<EC> * ue = m_effSelf->AsUlamElement();
    if (ue)
    {
      const T atom = this->ReadAtom();
      ue->Print(uc, bs, atom, printFlags, m_pos);
      return;
    }

    // If this isn't an ulam element, MFM doesn't have name info for
    // its type, but we can still print its class members
    m_effSelf->PrintClassMembers(uc, bs, m_stg, printFlags, m_pos);
  }

} //MFM
