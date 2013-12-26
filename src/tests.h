#ifndef TESTS_H
#define TESTS_H

#include "manhattandir_test.hpp"
#include "bitfield_test.hpp"
#include "point_test.hpp"
#include "p1atom_test.hpp"
#include "tile_test.hpp"
#include "grid_test.hpp"

int main(int argc, char** argv)
{
  PointTest::Test_pointAdd();
  PointTest::Test_pointMultiply();

  BitFieldTest::Test_bitfieldAllocate();
  BitFieldTest::Test_bitfieldSize();
  BitFieldTest::Test_bitfieldRead();
  BitFieldTest::Test_bitfieldWrite();
  BitFieldTest::Test_bitfieldInsert();
  BitFieldTest::Test_bitfieldRemove();

  ManhattanDirTest::Test_manhattandirConversion();

  P1AtomTest::Test_p1atomState();
  P1AtomTest::Test_p1atomLBCount();
  P1AtomTest::Test_p1atomReadBody();
  P1AtomTest::Test_p1atomAddLB();
  P1AtomTest::Test_p1atomAddSB();
  P1AtomTest::Test_p1atomRemoveLB();
  P1AtomTest::Test_p1atomRemoveSB();

  TileTest::Test_tilePlaceAtom();

  GridTest::Test_gridPlaceAtom();

  return 0;
}

#endif /*TESTS_H*/
