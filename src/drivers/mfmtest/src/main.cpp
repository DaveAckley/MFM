#include "main.h"

int main(int argc, char** argv)
{
  //  ManhattanDir<4>::ManhattanDir tables();

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

  EventWindowTest::Test_eventwindowConstruction();
  EventWindowTest::Test_eventwindowWrite();

  //ManhattanDir::DeallocTables();

  return 0;
}

