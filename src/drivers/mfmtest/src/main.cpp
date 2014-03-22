#include "main.h"

using namespace MFM;

int main(int argc, char** argv)
{

  Fail_test::Test_RunTests();
  FXP_test::Test_RunTests();
  ColorMapTest::Test_RunTests();
  RandomTest::Test_RunTests();

  PointTest::Test_pointAdd();
  PointTest::Test_pointMultiply();

  BitVectorTest::Test_bitVectorAllocate();
  BitVectorTest::Test_bitVectorSize();
  BitVectorTest::Test_bitVectorRead();
  BitVectorTest::Test_bitVectorWrite();
  //  BitVectorTest::Test_bitVectorInsert();
  //  BitVectorTest::Test_bitVectorRemove();

  MDistTest::Test_MDistConversion();

  P1AtomTest::Test_p1atomState();
  P1AtomTest::Test_p1atomLBCount();
  P1AtomTest::Test_p1atomReadBody();
  P1AtomTest::Test_p1atomAddLB();
  P1AtomTest::Test_p1atomAddSB();
  //  P1AtomTest::Test_p1atomRemoveLB();
  //  P1AtomTest::Test_p1atomRemoveSB();

  TileTest::Test_tilePlaceAtom();

  GridTest::Test_gridPlaceAtom();

  EventWindowTest::Test_eventwindowConstruction();
  EventWindowTest::Test_eventwindowWrite();

  return 0;
}

