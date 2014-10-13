#include "main.h"

using namespace MFM;

int main(int argc, char** argv)
{
  if (argc > 1)
  {
    MFM::LOG.SetByteSink(MFM::STDERR);
    MFM::LOG.SetLevel(MFM::LOG.ALL);
  }

  ElementRegistry_Test::Test_RunTests();
  ByteSource_Test::Test_RunTests();
  LineTailByteSink_Test::Test_RunTests();
  OverflowableCharBufferByteSink_Test::Test_RunTests();
  VArguments_Test::Test_RunTests();
  Logger_Test::Test_RunTests();
  UUID_Test::Test_RunTests();
  ByteSink_Test::Test_RunTests();
  Parity2D_4x4_Test::Test_RunTests();
  PSym_Test::Test_RunTests();

  Fail_Test::Test_RunTests();
  FXP_Test::Test_RunTests();
  ColorMap_Test::Test_RunTests();
  Random_Test::Test_RunTests();
  BitVector_Test::Test_RunTests();

  Point_Test::Test_pointAdd();
  Point_Test::Test_pointMultiply();

  MDist_Test::Test_MDistConversion();

#if 0  /* DEPRECATED */
  P1Atom_Test::Test_p1atomState();
  P1Atom_Test::Test_p1atomLBCount();
  P1Atom_Test::Test_p1atomReadBody();
  P1Atom_Test::Test_p1atomAddLB();
  P1Atom_Test::Test_p1atomAddSB();
#endif

  Tile_Test::Test_tilePlaceAtom();

  Grid_Test::Test_gridPlaceAtom();

  EventWindow_Test::Test_eventwindowConstruction();
  EventWindow_Test::Test_eventwindowWrite();

  ExternalConfig_Test::Test_RunTests();

  return 0;
}
