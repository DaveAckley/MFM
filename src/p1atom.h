#ifndef P1ATOM_H
#define P1ATOM_H

#define P1ATOM_SIZE 64

#define P1ATOM_HEADER_SIZE 16
#define P1ATOM_LONGBOND_SIZE 8
#define P1ATOM_SHORTBOND_SIZE 4

#include <stdio.h>
#include "itype.h"
#include "point.h"
#include "bitfield.h"

class P1Atom
{
private:
  BitField<P1ATOM_SIZE> m_bits;

public:
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

  void PrintBits(FILE* ostream)
  { m_bits.Print(ostream); }

  /* Adds a long bond. Returns its index. */
  u32 AddLongBond(Point<int>* offset);

  u32 AddShortBond(Point<int>* offset);

  /* 
   * Removes a long bond. Be careful; if a
   * bond is removed, the bonds ahead of it
   * (if they exist) will be pushed downwards.
   * The indices of these bonds will need to
   * be updated again afterwards.
   */
  void RemoveLongBond(u32 index);

  void RemoveShortBond(u32 index);
};

#endif /*P1ATOM_H*/
