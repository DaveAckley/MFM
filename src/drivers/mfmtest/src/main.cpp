#include "main.h"

using namespace MFM;

int main(int argc, char** argv)
{

  PSym_test::Test_RunTests();

  Fail_test::Test_RunTests();
  FXP_test::Test_RunTests();
  ColorMapTest::Test_RunTests();
  RandomTest::Test_RunTests();
  BitVectorTest::Test_RunTests();

  PointTest::Test_pointAdd();
  PointTest::Test_pointMultiply();

  MDistTest::Test_MDistConversion();

  P1AtomTest::Test_p1atomState();
  P1AtomTest::Test_p1atomLBCount();
  P1AtomTest::Test_p1atomReadBody();
  P1AtomTest::Test_p1atomAddLB();
  P1AtomTest::Test_p1atomAddSB();

  TileTest::Test_tilePlaceAtom();

  GridTest::Test_gridPlaceAtom();

  EventWindowTest::Test_eventwindowConstruction();
  EventWindowTest::Test_eventwindowWrite();

  return 0;
}

