#include "P1Atom.h"
#include "MDist.h"
#include <stdio.h>

namespace MFM {

  void printP1(const P1Atom & atom) 
  {
    atom.PrintBits(stdout);
    
    printf(",%d/%d/%d+%d",
           atom.GetLongBondCount(),atom.GetShortBondCount(),atom.GetStateBitCount(),
           atom.GetBitsAllocated());
    printf("\n");
  }

#if 0
u32 P1Atom::AddLongBond(const SPoint& offset)
{
  u32 newID = GetLongBondCount();
  
  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_LONG_BOND_SIZE,
		MDist4::get().FromPoint(offset, MANHATTAN_TABLE_LONG));

  SetLongBondCount(newID + 1);
  return newID;
}

u32 P1Atom::AddShortBond(const SPoint& offset)
{
  u32 newID = GetShortBondCount();

  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * GetLongBondCount() + 
    P1ATOM_SHORT_BOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_SHORT_BOND_SIZE,
		MDist4::get().FromPoint(offset, MANHATTAN_TABLE_SHORT));

  SetShortBondCount(newID + 1);
  return newID;
}
#endif

bool P1Atom::GetLongBond(u32 index, SPoint& pt) const
{
  if (index >= GetLongBondCount())
    return false;

  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * index;

  u8 bond = m_bits.Read(realIdx, 8);

  MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_LONG);
  return true;
}

bool P1Atom::GetShortBond(u32 index, SPoint& pt) const
{
  if (index >= GetShortBondCount())
    return false;

  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * GetLongBondCount() +
    P1ATOM_SHORT_BOND_SIZE * index;

  u8 bond = m_bits.Read(realIdx, 4);

  MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_SHORT);
  return true;
}

bool P1Atom::SetLongBond(u32 index, const SPoint& pt) 
{
  if (index >= GetLongBondCount())
    return false;

  s32 bond =  MDist4::get().FromPoint(pt,MANHATTAN_TABLE_LONG);
  if (bond < 0)
    return false;

  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * index;

  m_bits.Write(realIdx,8,bond);

  return true;
}

bool P1Atom::SetShortBond(u32 index, const SPoint& pt) 
{
  if (index >= GetShortBondCount())
    return false;

  s32 bond =  MDist4::get().FromPoint(pt,MANHATTAN_TABLE_SHORT);
  if (bond < 0)
    return false;

  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * GetLongBondCount() +
    P1ATOM_SHORT_BOND_SIZE * index;

  m_bits.Write(realIdx,4,bond);

  return true;
}


#if 0
void P1Atom::RemoveLongBond(u32 index)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * index;

  m_bits.Remove(realIdx, 8);

  SetLongBondCount(GetLongBondCount() - 1);
}
#endif

#if 0
void P1Atom::RemoveShortBond(u32 index)
{
  u32 realIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONG_BOND_SIZE * GetLongBondCount() +
    P1ATOM_SHORT_BOND_SIZE * index;

  m_bits.Remove(realIdx, 4);

  SetShortBondCount(GetShortBondCount() - 1);
}
#endif

P1Atom& P1Atom::operator=(const P1Atom & rhs)
{
  if (this == &rhs) return *this;

  m_bits = rhs.m_bits;

  /*

  int start;

  for(u32 i = 0;
      i < P1ATOM_SIZE / BitVector<P1ATOM_SIZE>::BITS_PER_UNIT; i++)
  {
    start = i * BitVector<P1ATOM_SIZE>::BITS_PER_UNIT;
    m_bits.Write(start,
                 BitVector<P1ATOM_SIZE>::BITS_PER_UNIT,
                 rhs.m_bits.Read(start,
                                 BitVector<P1ATOM_SIZE>::BITS_PER_UNIT
                                 ));
  }
  */

  return *this;
}
} /* namespace MFM */

