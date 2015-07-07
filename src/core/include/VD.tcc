/* -*- C++ -*- */
#include "Atom.h"

namespace MFM
{
  template <class AC>
  u32 VD::GetFieldAsBits(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    return a.m_bits.Read(start, length);
  }

  template <class AC>
  void VD::SetFieldAsBits(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const u32 val)
  {
    a.m_bits.Write(start, length, val);
  }

  template <class AC>
  u64 VD::GetLongFieldAsBits(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    return a.m_bits.ReadLong(start, length);
  }

  template <class AC>
  void VD::SetLongFieldAsBits(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const u64 val)
  {
    a.m_bits.WriteLong(start, length, val);
  }

  template <class AC>
  u64 VD::GetBitsAsU64(const typename AC::ATOM_TYPE & a) const
  {
    return GetLongFieldAsBits<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetBitsAsU64(typename AC::ATOM_TYPE & a, u64 val) const
  {
    SetLongFieldAsBits<AC>(m_length, m_start, a, val);
  }

  template <class AC>
  s32 VD::GetBitsAsS32(const typename AC::ATOM_TYPE & a) const
  {
    return (s32) GetFieldAsBits<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetBitsAsS32(typename AC::ATOM_TYPE & a, s32 val) const
  {
    if (val < m_min)
    {
      val = m_min;
    }
    else if (val > m_max)
    {
      val = m_max;
    }
    SetFieldAsBits<AC>(m_length, m_start, a, (u32) val);
  }

  template <class AC>
  u32 VD::GetFieldAsU32(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    return GetFieldAsBits<AC>(length, start, a);
  }

  template <class AC>
  void VD::SetFieldAsU32(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const u32 val)
  {
    SetFieldAsBits<AC>(length, start, a, val);
  }

  template <class AC>
  u32 VD::GetValueU32(const typename AC::ATOM_TYPE & a) const
  {
    AssertIsType(U32);
    return GetFieldAsBits<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetValueU32(typename AC::ATOM_TYPE & a, const u32 val) const
  {
    AssertIsType(U32);
    SetFieldAsBits<AC>(m_length, m_start, a, val);
  }

  template <class AC>
  s32 VD::GetFieldAsS32(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    const u32 SIGN_SHIFT = 32 - length;
    return ((s32) GetFieldAsU32<AC>(length, start, a)<<SIGN_SHIFT)>>SIGN_SHIFT;
  }

  template <class AC>
  void VD::SetFieldAsS32(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const s32 val)
  {
    SetFieldAsU32<AC>(length, start, a, (u32) val);
  }

  template <class AC>
  s32 VD::GetValueS32(const typename AC::ATOM_TYPE & a) const
  {
    AssertIsType(S32);
    return GetFieldAsS32<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetValueS32(typename AC::ATOM_TYPE & a, const s32 val) const
  {
    AssertIsType(S32);
    SetFieldAsS32<AC>(m_length, m_start, a, val);
  }


  template <class AC>
  bool VD::GetFieldAsBool(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    const u32 ones = __builtin_popcount(GetFieldAsU32<AC>(length, start, a));
    return ones > length - ones;
  }

  template <class AC>
  void VD::SetFieldAsBool(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const bool val)
  {
    SetFieldAsU32<AC>(length, start, a, val ? MakeMask(length) : 0u);
  }

  template <class AC>
  bool VD::GetValueBool(const typename AC::ATOM_TYPE & a) const
  {
    AssertIsType(BOOL);
    return GetFieldAsBool<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetValueBool(typename AC::ATOM_TYPE & a, const bool val) const
  {
    AssertIsType(BOOL);
    SetFieldAsBool<AC>(m_length, m_start, a, val);
  }

  template <class AC>
  u32 VD::GetFieldAsUnary(const u32 length, const u32 start, const typename AC::ATOM_TYPE & a)
  {
    return __builtin_popcount(GetFieldAsU32<AC>(length, start, a));
  }

  template <class AC>
  void VD::SetFieldAsUnary(const u32 length, const u32 start, typename AC::ATOM_TYPE & a, const u32 val)
  {
    SetFieldAsU32<AC>(length, start, a, MakeMask(val));
  }

  template <class AC>
  u32 VD::GetValueUnary(const typename AC::ATOM_TYPE & a) const
  {
    AssertIsType(UNARY);
    return GetFieldAsUnary<AC>(m_length, m_start, a);
  }

  template <class AC>
  void VD::SetValueUnary(typename AC::ATOM_TYPE & a, const u32 val) const
  {
    AssertIsType(UNARY);
    SetFieldAsUnary<AC>(m_length, m_start, a, val);
  }

  template <class AC>
  void VD::StoreValueByType(typename AC::ATOM_TYPE & a, const u32 val) const
  {
    switch(m_type) {
    case INVALID:
    default:
      FAIL(ILLEGAL_STATE);
    case U32:
      this->SetValueU32<AC>(a, val);
      break;
    case S32:
      this->SetValueS32<AC>(a, (s32) val);
      break;
    case BOOL:
      this->SetValueBool<AC>(a, val != 0);
      break;
    case UNARY:
      this->SetValueUnary<AC>(a, val);
      break;
    case BITS:
      this->SetBitsAsS32<AC>(a, (s32) val);
      break;
    }
  }

  template <class AC>
  s32 VD::LoadValueByType(typename AC::ATOM_TYPE & a) const
  {
    switch(m_type) {
    case INVALID:
    default:
      FAIL(ILLEGAL_STATE);
    case U32:
      return this->GetValueU32<AC>(a);
    case S32:
      return (s32) this->GetValueS32<AC>(a);
    case BOOL:
      return this->GetValueBool<AC>(a) ? 1 : 0;
    case UNARY:
      return this->GetValueUnary<AC>(a);
    case BITS:
      return this->GetBitsAsS32<AC>(a);
    }
  }
}
