/* -*- C++ -*- */
#include "Atom.h"

namespace MFM
{
  template <class T>
  u32 VD::GetFieldAsBits(const u32 length, const u32 start, const T& a)
  {
    return a.m_bits.Read(start, length);
  }

  template <class T>
  void VD::SetFieldAsBits(const u32 length, const u32 start, T& a, const u32 val)
  {
    a.m_bits.Write(start, length, val);
  }

  template <class T>
  u64 VD::GetLongFieldAsBits(const u32 length, const u32 start, const T& a)
  {
    return a.m_bits.ReadLong(start, length);
  }

  template <class T>
  void VD::SetLongFieldAsBits(const u32 length, const u32 start, T& a, const u64 val)
  {
    a.m_bits.WriteLong(start, length, val);
  }

  template <class T>
  u64 VD::GetBitsAsU64(const T& a) const
  {
    return GetLongFieldAsBits<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetBitsAsU64(T& a, u64 val) const
  {
    SetLongFieldAsBits<T>(m_length, m_start, a, val);
  }

  template <class T>
  s32 VD::GetBitsAsS32(const T& a) const
  {
    return (s32) GetFieldAsBits<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetBitsAsS32(T& a, s32 val) const
  {
    if (val < m_min)
    {
      val = m_min;
    }
    else if (val > m_max)
    {
      val = m_max;
    }
    SetFieldAsBits<T>(m_length, m_start, a, (u32) val);
  }

  template <class T>
  u32 VD::GetFieldAsU32(const u32 length, const u32 start, const T& a)
  {
    return GetFieldAsBits<T>(length, start, a);
  }

  template <class T>
  void VD::SetFieldAsU32(const u32 length, const u32 start, T& a, const u32 val)
  {
    SetFieldAsBits<T>(length, start, a, val);
  }

  template <class T>
  u32 VD::GetValueU32(const T& a) const
  {
    AssertIsType(U32);
    return GetFieldAsBits<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetValueU32(T& a, const u32 val) const
  {
    AssertIsType(U32);
    SetFieldAsBits<T>(m_length, m_start, a, val);
  }

  template <class T>
  s32 VD::GetFieldAsS32(const u32 length, const u32 start, const T& a)
  {
    const u32 SIGN_SHIFT = 32 - length;
    return ((s32) GetFieldAsU32<T>(length, start, a)<<SIGN_SHIFT)>>SIGN_SHIFT;
  }

  template <class T>
  void VD::SetFieldAsS32(const u32 length, const u32 start, T& a, const s32 val)
  {
    SetFieldAsU32<T>(length, start, a, (u32) val);
  }

  template <class T>
  s32 VD::GetValueS32(const T& a) const
  {
    AssertIsType(S32);
    return GetFieldAsS32<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetValueS32(T& a, const s32 val) const
  {
    AssertIsType(S32);
    SetFieldAsS32<T>(m_length, m_start, a, val);
  }


  template <class T>
  bool VD::GetFieldAsBool(const u32 length, const u32 start, const T& a)
  {
    const u32 ones = __builtin_popcount(GetFieldAsU32<T>(length, start, a));
    return ones > length - ones;
  }

  template <class T>
  void VD::SetFieldAsBool(const u32 length, const u32 start, T& a, const bool val)
  {
    SetFieldAsU32<T>(length, start, a, val ? MakeMask(length) : 0u);
  }

  template <class T>
  bool VD::GetValueBool(const T& a) const
  {
    AssertIsType(BOOL);
    return GetFieldAsBool<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetValueBool(T& a, const bool val) const
  {
    AssertIsType(BOOL);
    SetFieldAsBool<T>(m_length, m_start, a, val);
  }

  template <class T>
  u32 VD::GetFieldAsUnary(const u32 length, const u32 start, const T& a)
  {
    return __builtin_popcount(GetFieldAsU32<T>(length, start, a));
  }

  template <class T>
  void VD::SetFieldAsUnary(const u32 length, const u32 start, T& a, const u32 val)
  {
    SetFieldAsU32<T>(length, start, a, MakeMask(val));
  }

  template <class T>
  u32 VD::GetValueUnary(const T& a) const
  {
    AssertIsType(UNARY);
    return GetFieldAsUnary<T>(m_length, m_start, a);
  }

  template <class T>
  void VD::SetValueUnary(T& a, const u32 val) const
  {
    AssertIsType(UNARY);
    SetFieldAsUnary<T>(m_length, m_start, a, val);
  }

  template <class T>
  void VD::StoreValueByType(T & a, const u32 val) const
  {
    switch(m_type) {
    case INVALID:
    default:
      FAIL(ILLEGAL_STATE);
    case U32:
      this->SetValueU32<T>(a, val);
      break;
    case S32:
      this->SetValueS32<T>(a, (s32) val);
      break;
    case BOOL:
      this->SetValueBool<T>(a, val != 0);
      break;
    case UNARY:
      this->SetValueUnary<T>(a, val);
      break;
    case BITS:
      this->SetBitsAsS32<T>(a, (s32) val);
      break;
    }
  }

  template <class T>
  s32 VD::LoadValueByType(T & a) const
  {
    switch(m_type) {
    case INVALID:
    default:
      FAIL(ILLEGAL_STATE);
    case U32:
      return this->GetValueU32<T>(a);
    case S32:
      return (s32) this->GetValueS32<T>(a);
    case BOOL:
      return this->GetValueBool<T>(a) ? 1 : 0;
    case UNARY:
      return this->GetValueUnary<T>(a);
    case BITS:
      return this->GetBitsAsS32<T>(a);
    }
  }
}
