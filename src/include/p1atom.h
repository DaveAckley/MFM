#ifndef P1ATOM_H      /* -*- C++ -*- */
#define P1ATOM_H

#define P1ATOM_SIZE 64

#define P1ATOM_HEADER_SIZE 16
#define P1ATOM_LONGBOND_SIZE 8
#define P1ATOM_SHORTBOND_SIZE 4

#include <stdio.h>
#include "itype.h"
#include "point.h"
#include "bitfield.h"
#include "manhattandir.h"


class P1Atom
{
private:
  BitField<P1ATOM_SIZE> m_bits;

  typedef ManhattanDir<4> ManhattanDir4;

public:

  static u32 StateFunc(P1Atom* atom)
  {
    return atom->GetState();
  }

  P1Atom() { }

  P1Atom(u32 state)
  { SetState(state); }

  u32 GetState()
  { return m_bits.Read(2, 6); }

  void SetState(u32 state)
  { m_bits.Write(2, 6, state); }

  u32 GetLongBondCount()
  { return m_bits.Read(9, 3); }

  void SetLongBondCount(u32 count)
  { m_bits.Write(9, 3, count); }

  u32 GetShortBondCount()
  { return m_bits.Read(12, 4); }

  void SetShortBondCount(u32 count)
  { m_bits.Write(12, 4, count); }

  void ReadVariableBodyInto(u32* arr)
  {
    arr[0] = m_bits.Read(16, 16);
    arr[1] = m_bits.Read(32, 32);
  }

  void WriteVariableBodyFrom(u32* arr)
  {
    m_bits.Write(16, 16, arr[0]);
    m_bits.Write(32, 32, arr[1]);
  }

  void WriteLowerBits(u32 val)
  {
    m_bits.Write(32, 32, val);
  }

  u32 ReadLowerBits()
  {
    return m_bits.Read(32, 32);
  }

  void PrintBits(FILE* ostream)
  { m_bits.Print(ostream); }

  /* Adds a long bond. Returns its index. */
  u32 AddLongBond(Point<int>& offset);

  u32 AddShortBond(Point<int>& offset);

  /* Fills pt with the long bond location in index. */
  void FillLongBond(u32 index, Point<int>& pt);

  void FillShortBond(u32 index, Point<int>& pt);

  /* 
   * Removes a long bond. Be careful; if a
   * bond is removed, the bonds ahead of it
   * (if they exist) will be pushed downwards.
   * The indices of these bonds will need to
   * be updated again afterwards.
   */
  void RemoveLongBond(u32 index);

  void RemoveShortBond(u32 index);

  P1Atom& operator=(P1Atom rhs);
};

#endif /*P1ATOM_H*/
