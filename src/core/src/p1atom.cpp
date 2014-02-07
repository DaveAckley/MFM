#include "p1atom.h"
#include "manhattandir.h"

namespace MFM {

u32 P1Atom::AddLongBond(const SPoint& offset)
{
  u32 newID = GetLongBondCount();
  
  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_LONGBOND_SIZE,
		ManhattanDir4::get().FromPoint(offset, MANHATTAN_TABLE_LONG));

  SetLongBondCount(newID + 1);
  return newID;
}

u32 P1Atom::AddShortBond(const SPoint& offset)
{
  u32 newID = GetShortBondCount();

  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * GetLongBondCount() + 
    P1ATOM_SHORTBOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_SHORTBOND_SIZE,
		ManhattanDir4::get().FromPoint(offset, MANHATTAN_TABLE_SHORT));

  SetShortBondCount(newID + 1);
  return newID;
}

void P1Atom::FillLongBond(u32 index, SPoint& pt)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * index;

  u8 bond = m_bits.Read(realIdx, 8);

  ManhattanDir4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_LONG);
}

void P1Atom::FillShortBond(u32 index, SPoint& pt)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * GetLongBondCount() +
    P1ATOM_SHORTBOND_SIZE * index;

  u8 bond = m_bits.Read(realIdx, 4);

  ManhattanDir4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_SHORT);
}

void P1Atom::RemoveLongBond(u32 index)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * index;

  m_bits.Remove(realIdx, 8);

  SetLongBondCount(GetLongBondCount() - 1);
}

void P1Atom::RemoveShortBond(u32 index)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * GetLongBondCount() +
    P1ATOM_SHORTBOND_SIZE * index;

  m_bits.Remove(realIdx, 4);

  SetShortBondCount(GetShortBondCount() - 1);
}

P1Atom& P1Atom::operator=(P1Atom rhs)
{
  int start;
  for(u32 i = 0;
      i < P1ATOM_SIZE / BitField<P1ATOM_SIZE>::BITS_PER_UNIT; i++)
  {
    start = i * BitField<P1ATOM_SIZE>::BITS_PER_UNIT;
    m_bits.Write(start,
		 BitField<P1ATOM_SIZE>::BITS_PER_UNIT,
		 rhs.m_bits.Read(start,
				 BitField<P1ATOM_SIZE>::BITS_PER_UNIT
				 ));
  }
  return *this;
}
} /* namespace MFM */

