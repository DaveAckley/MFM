#include "P2Atom.h"
#include "MDist.h"

namespace MFM {

#if 0
  u32 P2Atom::AddLongBond(const SPoint& offset)
  {
    u32 newID = GetLongBondCount();

    u32 newBondIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * newID;

    m_bits.Insert(newBondIdx, P2ATOM_LONGBOND_SIZE,
                  MDist4::get().FromPoint(offset, MANHATTAN_TABLE_LONG));

    SetLongBondCount(newID + 1);
    return newID;
  }

  u32 P2Atom::AddShortBond(const SPoint& offset)
  {
    u32 newID = GetShortBondCount();

    u32 newBondIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * GetLongBondCount() +
      P2ATOM_SHORTBOND_SIZE * newID;

    m_bits.Insert(newBondIdx, P2ATOM_SHORTBOND_SIZE,
                  MDist4::get().FromPoint(offset, MANHATTAN_TABLE_SHORT));

    SetShortBondCount(newID + 1);
    return newID;
  }
#endif

  void P2Atom::FillLongBond(u32 index, SPoint& pt)
  {
    u32 realIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * index;

    u8 bond = m_bits.Read(realIdx, 8);

    MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_LONG);
  }

  void P2Atom::FillShortBond(u32 index, SPoint& pt)
  {
    u32 realIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * GetLongBondCount() +
      P2ATOM_SHORTBOND_SIZE * index;

    u8 bond = m_bits.Read(realIdx, 4);

    MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_SHORT);
  }

#if 0
  void P2Atom::RemoveLongBond(u32 index)
  {
    u32 realIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * index;

    m_bits.Remove(realIdx, 8);

    SetLongBondCount(GetLongBondCount() - 1);
  }

  void P2Atom::RemoveShortBond(u32 index)
  {
    u32 realIdx = P2ATOM_HEADER_SIZE +
      P2ATOM_LONGBOND_SIZE * GetLongBondCount() +
      P2ATOM_SHORTBOND_SIZE * index;

    m_bits.Remove(realIdx, 4);

    SetShortBondCount(GetShortBondCount() - 1);
  }
#endif

  P2Atom& P2Atom::operator=(P2Atom rhs)
  {
    int start;
    for(u32 i = 0;
        i < P2ATOM_SIZE / BitVector<P2ATOM_SIZE>::BITS_PER_UNIT; i++)
      {
        start = i * BitVector<P2ATOM_SIZE>::BITS_PER_UNIT;
        m_bits.Write(start,
                     BitVector<P2ATOM_SIZE>::BITS_PER_UNIT,
                     rhs.m_bits.Read(start,
                                     BitVector<P2ATOM_SIZE>::BITS_PER_UNIT
                                     ));
      }
    return *this;
  }
} /* namespace MFM */
