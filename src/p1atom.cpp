#include "p1atom.h"
#include "manhattandir.h"

u32 P1Atom::AddLongBond(Point<int>* offset)
{
  u32 newID = GetLongBondCount();
  
  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_LONGBOND_SIZE,
		ManhattanDir::FromPoint(offset));

  SetLongBondCount(newID + 1);
  return newID;
}

u32 P1Atom::AddShortBond(Point<int>* offset)
{
  u32 newID = GetShortBondCount();

  u32 newBondIdx = P1ATOM_HEADER_SIZE +
    P1ATOM_LONGBOND_SIZE * GetLongBondCount() + 
    P1ATOM_SHORTBOND_SIZE * newID;

  m_bits.Insert(newBondIdx, P1ATOM_SHORTBOND_SIZE,
		ManhattanDir::FromPoint(offset));

  SetShortBondCount(newID + 1);
  return newID;
}

void P1Atom::RemoveLongBond(u32 index)
{
  int bondsToMove;
  
  
  SetShortBondCount(GetShortBondCount() - 1);
}

void P1Atom::RemoveShortBond(u32 index)
{

}
