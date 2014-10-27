/* -*- C++ -*- */
#include "Atom.h"

namespace MFM
{
  template <class CC>
  u32 VD::GetFieldAsBits(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    return a.m_bits.Read(start, length);
  }

  template <class CC>
  void VD::SetFieldAsBits(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const u32 val)
  {
    a.m_bits.Write(start, length, val);
  }

  template <class CC>
  u64 VD::GetLongFieldAsBits(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    return a.m_bits.ReadLong(start, length);
  }

  template <class CC>
  void VD::SetLongFieldAsBits(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const u64 val)
  {
    a.m_bits.WriteLong(start, length, val);
  }

  template <class CC>
  u64 VD::GetBitsAsU64(const typename CC::ATOM_TYPE & a) const
  {
    return GetLongFieldAsBits<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetBitsAsU64(typename CC::ATOM_TYPE & a, u64 val) const
  {
    SetLongFieldAsBits<CC>(m_length, m_start, a, val);
  }

  template <class CC>
  s32 VD::GetBitsAsS32(const typename CC::ATOM_TYPE & a) const
  {
    return (s32) GetFieldAsBits<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetBitsAsS32(typename CC::ATOM_TYPE & a, s32 val) const
  {
    if (val < m_min)
    {
      val = m_min;
    }
    else if (val > m_max)
    {
      val = m_max;
    }
    SetFieldAsBits<CC>(m_length, m_start, a, (u32) val);
  }

  template <class CC>
  u32 VD::GetFieldAsU32(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    return GetFieldAsBits<CC>(length, start, a);
  }

  template <class CC>
  void VD::SetFieldAsU32(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const u32 val)
  {
    SetFieldAsBits<CC>(length, start, a, val);
  }

  template <class CC>
  u32 VD::GetValueU32(const typename CC::ATOM_TYPE & a) const
  {
    AssertIsType(U32);
    return GetFieldAsBits<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetValueU32(typename CC::ATOM_TYPE & a, const u32 val) const
  {
    AssertIsType(U32);
    SetFieldAsBits<CC>(m_length, m_start, a, val);
  }

  template <class CC>
  s32 VD::GetFieldAsS32(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    const u32 SIGN_SHIFT = 32 - length;
    return ((s32) GetFieldAsU32<CC>(length, start, a)<<SIGN_SHIFT)>>SIGN_SHIFT;
  }

  template <class CC>
  void VD::SetFieldAsS32(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const s32 val)
  {
    SetFieldAsU32<CC>(length, start, a, (u32) val);
  }

  template <class CC>
  s32 VD::GetValueS32(const typename CC::ATOM_TYPE & a) const
  {
    AssertIsType(S32);
    return GetFieldAsS32<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetValueS32(typename CC::ATOM_TYPE & a, const s32 val) const
  {
    AssertIsType(S32);
    SetFieldAsS32<CC>(m_length, m_start, a, val);
  }


  template <class CC>
  bool VD::GetFieldAsBool(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    const u32 ones = __builtin_popcount(GetFieldAsU32<CC>(length, start, a));
    return ones > length - ones;
  }

  template <class CC>
  void VD::SetFieldAsBool(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const bool val)
  {
    SetFieldAsU32<CC>(length, start, a, val ? MakeMask(length) : 0u);
  }

  template <class CC>
  bool VD::GetValueBool(const typename CC::ATOM_TYPE & a) const
  {
    AssertIsType(BOOL);
    return GetFieldAsBool<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetValueBool(typename CC::ATOM_TYPE & a, const bool val) const
  {
    AssertIsType(BOOL);
    SetFieldAsBool<CC>(m_length, m_start, a, val);
  }

  template <class CC>
  u32 VD::GetFieldAsUnary(const u32 length, const u32 start, const typename CC::ATOM_TYPE & a)
  {
    return __builtin_popcount(GetFieldAsU32<CC>(length, start, a));
  }

  template <class CC>
  void VD::SetFieldAsUnary(const u32 length, const u32 start, typename CC::ATOM_TYPE & a, const u32 val)
  {
    SetFieldAsU32<CC>(length, start, a, MakeMask(val));
  }

  template <class CC>
  u32 VD::GetValueUnary(const typename CC::ATOM_TYPE & a) const
  {
    AssertIsType(UNARY);
    return GetFieldAsUnary<CC>(m_length, m_start, a);
  }

  template <class CC>
  void VD::SetValueUnary(typename CC::ATOM_TYPE & a, const u32 val) const
  {
    AssertIsType(UNARY);
    SetFieldAsUnary<CC>(m_length, m_start, a, val);
  }

}
