/* -*- C++ -*- */
#include "Fail.h"
#include "BitVector.h"

namespace MFM {

  template <class EC>
  UlamRef<EC>::UlamRef(u32 pos, u32 len, BitStorage<EC>& stg, const UlamClass<EC> * effself)
    : m_effSelf(effself)
    , m_stg(stg)
    , m_pos(pos)
    , m_len(len)
  {
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef & existing, s32 pos, u32 len, const UlamClass<EC> * effself)
    : m_effSelf(effself)
    , m_stg(existing.m_stg)
    , m_len(len)
  {
    s32 newpos = pos + (s32) existing.GetPos(); //e.g. pos -25 to start of atom of element ref
    MFM_API_ASSERT_ARG(newpos >= 0); //non-negative
    m_pos = (u32) newpos; //save as unsigned
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
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
