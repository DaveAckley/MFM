/* -*- C++ -*- */

#include "MDist.h"
#include <stdio.h>

namespace MFM {

  template<class PC>
  void printP1(const P1Atom<PC> & atom)
  {
    atom.PrintBits(stdout);

    LOG.Debug(",%d/%d/%d+%d\n",
	      atom.GetLongBondCount(),atom.GetShortBondCount(),atom.GetStateBitCount(),
	      atom.GetBitsAllocated());
  }

  template<class PC>
  bool P1Atom<PC>::GetLongBond(u32 index, SPoint& pt) const
  {
    if (index >= GetLongBondCount())
      return false;

    u32 realIdx = P1ATOM_HEADER_SIZE +
      P1ATOM_LONG_BOND_SIZE * index;

    u8 bond = this->m_bits.Read(realIdx, 8);

    MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_LONG);
    return true;
  }

  template<class PC>
  bool P1Atom<PC>::GetShortBond(u32 index, SPoint& pt) const
  {
    if (index >= GetShortBondCount())
      return false;

    u32 realIdx = P1ATOM_HEADER_SIZE +
      P1ATOM_LONG_BOND_SIZE * GetLongBondCount() +
      P1ATOM_SHORT_BOND_SIZE * index;

    u8 bond = this->m_bits.Read(realIdx, 4);

    MDist4::get().FillFromBits(pt, bond, MANHATTAN_TABLE_SHORT);
    return true;
  }

  template<class PC>
  bool P1Atom<PC>::SetLongBond(u32 index, const SPoint& pt)
  {
    if (index >= GetLongBondCount())
      return false;

    s32 bond =  MDist4::get().FromPoint(pt,MANHATTAN_TABLE_LONG);
    if (bond < 0)
      return false;

    u32 realIdx = P1ATOM_HEADER_SIZE +
      P1ATOM_LONG_BOND_SIZE * index;

    this->m_bits.Write(realIdx,8,bond);

    return true;
  }

  template<class PC>
  bool P1Atom<PC>::SetShortBond(u32 index, const SPoint& pt)
  {
    if (index >= GetShortBondCount())
      return false;

    s32 bond =  MDist4::get().FromPoint(pt,MANHATTAN_TABLE_SHORT);
    if (bond < 0)
      return false;

    u32 realIdx = P1ATOM_HEADER_SIZE +
      P1ATOM_LONG_BOND_SIZE * GetLongBondCount() +
      P1ATOM_SHORT_BOND_SIZE * index;

    this->m_bits.Write(realIdx,4,bond);

    return true;
  }

  template<class PC>
  P1Atom<PC>& P1Atom<PC>::operator=(const P1Atom<PC> & rhs)
  {
    if (this == &rhs) return *this;

    this->m_bits = rhs.m_bits;

    return *this;
  }
} /* namespace MFM */
