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

} //MFM
