#include "assert.h"
#include "eventwindow.h"
#include "p1atom_test.hpp"
#include "p1atom.h"
#include "manhattandir.h"



void P1AtomTest::Test_p1atomState()
{
  P1Atom atom(32);

  assert(atom.GetState() == 32);
  
  atom.SetState(15);
  
  assert(atom.GetState() == 15);
}

void P1AtomTest::Test_p1atomLBCount()
{
  P1Atom atom;
  Point<int> offset(1, -1);

  atom.AddLongBond(offset);

  assert(atom.GetLongBondCount() == 1);
}

void P1AtomTest::Test_p1atomSBCount()
{
  P1Atom atom;
  Point<int> offset(1, -1);

  atom.AddShortBond(offset);
  atom.AddShortBond(offset);

  assert(atom.GetShortBondCount() == 2);
}

void P1AtomTest::Test_p1atomReadBody()
{

  u32 blocks[2];
  P1Atom atom;

  Point<int> points[8];
  u8 pvals[8];
  
  points[0].Set(0, 1);
  points[1].Set(0, 2);
  points[2].Set(-2, 0);
  points[3].Set(-1, -1);
  points[4].Set(2, 1);
  points[5].Set(2, 0);
  points[6].Set(-2, 0);
  points[7].Set(1, 3);

  ManhattanDir::AllocTables(EVENT_WINDOW_RADIUS);

  for(int i = 0; i < 8; i++)
  {
    TableType type = (i < 4) ? 
      MANHATTAN_TABLE_SHORT : MANHATTAN_TABLE_LONG;

    pvals[i] = ManhattanDir::FromPoint(points[i], type);
  }

  for(int i = 0; i < 8; i++)
  {
    if(i < 4)
    {
      atom.AddShortBond(points[i]);
    }
    else
    {
      atom.AddLongBond(points[i]);
    }
  }

  atom.ReadVariableBodyInto(blocks);
  
  BitField<64> bf(blocks);

  bf.Remove(0, 16);

  for(int i = 0; i < 4; i++)
  {
    assert(bf.Read(i * 8, 8) == pvals[i + 4]);
    assert(bf.Read(32 + i * 4, 4) == pvals[i]);
  }
  
}

void P1AtomTest::Test_p1atomAddLB()
{
  Point<int> lbonds[4];
  P1Atom atom;
  
  lbonds[0].Set(0, 1);
  lbonds[1].Set(0, 2);
  lbonds[2].Set(0, 3);
  lbonds[3].Set(-4, 0);

  for(int i = 0; i < 4; i++)
  {
    atom.AddLongBond(lbonds[i]);
  }

  assert(atom.GetLongBondCount() == 4);
}

void P1AtomTest::Test_p1atomAddSB()
{
  Point<int> sbond(1, 1);
  Point<int> out;
  P1Atom atom(32);

  for(int i = 0; i < 10; i++)
  {
    atom.AddShortBond(sbond);
  }

  assert(atom.GetShortBondCount() == 10);

  for(int i = 0; i < 10; i++)
  {
    atom.FillShortBond(i, out);
    assert(out == sbond);
  }
}

void P1AtomTest::Test_p1atomRemoveLB()
{
  Point<int> lbond1(2, 2);
  Point<int> lbond2(3, 1);
  P1Atom atom(12);

  atom.AddLongBond(lbond1);
  atom.AddLongBond(lbond2);

  atom.RemoveLongBond(0);

  assert(atom.GetLongBondCount() == 1);

  u32 bits[2];
  atom.ReadVariableBodyInto(bits);
  
  BitField<64> bf(bits);

  bf.Remove(0, 16);
  
  assert(bf.Read(8, 8) == 0);
  assert(bf.Read(16, 32) == 0);
  
}

void P1AtomTest::Test_p1atomRemoveSB()
{
  Point<int> sb1(0, 2);
  Point<int> sb2(1, -1);
  P1Atom atom(9);

  atom.AddShortBond(sb1);
  atom.AddShortBond(sb2);

  atom.RemoveShortBond(0);

  assert(atom.GetShortBondCount() == 1);

  u32 bits[2];
  atom.ReadVariableBodyInto(bits);

  BitField<64> bf(bits);

  bf.Remove(0, 16);

  assert(bf.Read(4, 4) == 0);
  assert(bf.Read(8, 8) == 0);
  assert(bf.Read(16, 32) == 0);

  atom.RemoveShortBond(0);

  assert(atom.GetShortBondCount() == 0);

  atom.ReadVariableBodyInto(bits);

  assert(bits[0] == 0);
  assert(bits[1] == 0);

  atom.AddLongBond(sb1);
  atom.AddShortBond(sb2);
  atom.AddShortBond(sb1);

  atom.RemoveShortBond(0);

  atom.ReadVariableBodyInto(bits);

  assert((bits[0] & 0xf) == 0);
  assert(bits[1] == 0);
}
