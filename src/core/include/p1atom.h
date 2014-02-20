#ifndef P1ATOM_H      /* -*- C++ -*- */
#define P1ATOM_H

#include <stdio.h>
#include "itype.h"
#include "point.h"
#include "BitVector.h"
#include "manhattandir.h"
#include "atom.h"

#define P1ATOM_SIZE 64

#define P1ATOM_HEADER_SIZE 16
#define P1ATOM_LONG_BOND_SIZE 8
#define P1ATOM_SHORT_BOND_SIZE 4


namespace MFM {

class P1Atom : public Atom<P1Atom,4>
{
private:
  BitVector<P1ATOM_SIZE> m_bits;

  typedef ManhattanDir<4> ManhattanDir4;

  /* We really don't want to allow the public to change the type of a
     p1atom, since the type doesn't mean much without the atomic
     header as well */

  void SetType(u32 type, u32 width) 
  {
    m_bits.Write(P1ATOM_SIZE-width, width, type); 
  }

public:

  virtual u32 GetType() const {
    s32 bitsUsed = GetBitsAllocated();
    if (bitsUsed > P1ATOM_SIZE)
      FAIL(ILLEGAL_STATE);

    // The up-to 31 bits left over after everything else are the type bits
    u32 typeBits = P1ATOM_SIZE-bitsUsed;
    if (typeBits > 31) typeBits = 31;
    return m_bits.Read(P1ATOM_SIZE-typeBits,typeBits);
  }

  /*
  static u32 StateFunc(P1Atom* atom)
  {
    return atom->GetState();
  }
  */

  P1Atom()
  {
    InitAtom(ELEMENT_NOTHING,0,0,48);
  }

  P1Atom(u32 type, u32 longc, u32 shortc, u32 statec) 
  { 
    InitAtom(type,longc,shortc,statec);
  }

  void InitAtom(u32 type, u32 longc, u32 shortc, u32 statec) 
  {
    SetLongBondCount(longc);
    SetShortBondCount(shortc);
    SetStateBitCount(statec);
    u32 used = GetBitsAllocated();

    if (used > P1ATOM_SIZE)
      FAIL(ILLEGAL_ARGUMENT);

    u32 avail = P1ATOM_SIZE - used;
    if (avail > 31) avail = 31;

    if (type >= 1u<<avail)
      FAIL(ILLEGAL_ARGUMENT);
    SetType(type, avail);
  }

  u32 GetBitsAllocated() const
  {
    return
      P1ATOM_HEADER_SIZE+
      GetLongBondCount()*P1ATOM_LONG_BOND_SIZE
      + GetShortBondCount()*P1ATOM_SHORT_BOND_SIZE
      + GetStateBitCount();
  }

  u32 GetStateBitCount() const
  { return m_bits.Read(2, 6); }

  void SetStateBitCount(u32 state)
  { m_bits.Write(2, 6, state); }

  u32 GetStateField(u32 stateIndex, u32 stateWidth) const
  {
    // The state bits end at the end of the 'allocated bits'
    u32 lastState = GetBitsAllocated();
    return m_bits.Read(lastState-stateWidth, stateWidth);
  }

  void SetStateField(u32 stateIndex, u32 stateWidth, u32 value) 
  {
    u32 lastState = GetBitsAllocated();
    return m_bits.Write(lastState-stateWidth, stateWidth, value);
  }

  u32 GetLongBondCount() const
  { return m_bits.Read(9, 3); }

  void SetLongBondCount(u32 count)
  { m_bits.Write(9, 3, count); }

  u32 GetShortBondCount() const
  { return m_bits.Read(12, 4); }

  void SetShortBondCount(u32 count)
  { m_bits.Write(12, 4, count); }

  u32 GetLongBond(u32 index) const {
    if (index>=GetLongBondCount()) return 0;
    return m_bits.Read(16+index*8,8);
  }
  void SetLongBond(u32 index, u8 value) {
    if (index>=GetLongBondCount()) return;
    return m_bits.Write(16+index*8,8,(u32) value);
  }

  u32 GetShortBond(u32 index) const {
    if (index>=GetShortBondCount()) return 0;
    u32 start = GetLongBondCount()*8+16;
    return m_bits.Read(start+index*4,4);
  }
  void SetShortBond(u32 index, u8 value) {
    if (index>=GetLongBondCount()) return;
    u32 start = GetLongBondCount()*8+16;
    return m_bits.Write(start+index*4,4,(u32) value);
  }

  void ReadVariableBodyInto(u32* arr) const
  {
    arr[0] = m_bits.Read(16, 16);
    arr[1] = m_bits.Read(32, 32);
  }

  void WriteVariableBodyFrom(u32* arr)
  {
    m_bits.Write(16, 16, arr[0]);
    m_bits.Write(32, 32, arr[1]);
  }

  /*
  void WriteLowerBits(u32 val)
  {
    m_bits.Write(32, 32, val);
  }

  u32 ReadLowerBits() const
  {
    return m_bits.Read(32, 32);
  }
  */

  void PrintBits(FILE* ostream) const
  { m_bits.Print(ostream); }

#if 0
  /* Adds a long bond. Returns its index. */
  u32 AddLongBond(const SPoint& offset);

  u32 AddShortBond(const SPoint& offset);
#endif

  /**
   * Fills pt with the long bond location in index and returns true,
   * if atom has at least index+1 long bonds.  Otherwise returns false
   * and pt is unchanged */
  bool GetLongBond(u32 index, SPoint& pt) const;

  /**
   * Fills pt with the short bond location in index and returns true,
   * if atom has at least index+1 short bonds.  Otherwise returns
   * false and pt is unchanged */
  bool GetShortBond(u32 index, SPoint& pt) const;

  /**
   * Stores pt in the long bond location index and returns true, if
   * atom has at least index+1 long bonds, and pt is representable as
   * a long bond.  Otherwise returns false and *this is unchanged */
  bool SetLongBond(u32 index, const SPoint& pt);

  /**
   * Stores pt in the short bond location in index and returns true,
   * if atom has at least index+1 short bonds, and pt is representable
   * as a short bond.  Otherwise returns false and *this is unchanged */
  bool SetShortBond(u32 index, const SPoint& pt);

  /* 
   * Removes a long bond. Be careful; if a
   * bond is removed, the bonds ahead of it
   * (if they exist) will be pushed downwards.
   * The indices of these bonds will need to
   * be updated again afterwards.
   */
  void RemoveLongBond(u32 index);

  void RemoveShortBond(u32 index);

  P1Atom& operator=(const P1Atom & rhs);
};
} /* namespace MFM */
#endif /*P1ATOM_H*/

