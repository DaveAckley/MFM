/* -*- C++ -*- */

namespace MFM {

  template <class EC>
  UlamRefMutable<EC>::UlamRefMutable()
    : m_ucptr(NULL)
    , m_effSelf(NULL)
    , m_stgptr(NULL)
    , m_pos(0)
    , m_len(0)
    , m_usage(UlamRef<EC>::PRIMITIVE)
    , m_posToEff(0)
  { }

  template <class EC>
  UlamRefMutable<EC>& UlamRefMutable<EC>::operator=(const UlamRef<EC>& rhs)
    {
      m_ucptr = rhs.GetContextAsPointer();
      m_effSelf = rhs.GetEffectiveSelfPointer();
      m_stgptr = & rhs.GetStorage();
      m_pos = rhs.GetPos();
      m_len = rhs.GetLen();
      m_usage = rhs.GetUsage();
      m_posToEff = rhs.GetPosToEffectiveSelf();
      return *this;
    }

} //MFM
