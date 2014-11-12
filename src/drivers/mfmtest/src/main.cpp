#include "main.h"

using namespace MFM;

#define TEST(className)                             \
  do {                                              \
    MFM::LOG.Message("Starting " # className);      \
    className::Test_RunTests();                     \
    MFM::LOG.Message("Done     " # className);      \
  } while (0)

int main(int argc, char** argv)
{
  if (argc > 1)
  {
    MFM::LOG.SetByteSink(MFM::STDERR);
    MFM::LOG.SetLevel(MFM::LOG.ALL);
  }

  TEST(GridTransceiver_Test);
  TEST(ElementRegistry_Test);
  TEST(ByteSource_Test);
  TEST(LineTailByteSink_Test);
  TEST(OverflowableCharBufferByteSink_Test);
  TEST(VArguments_Test);
  TEST(Logger_Test);
  TEST(UUID_Test);
  TEST(ByteSink_Test);
  TEST(Parity2D_4x4_Test);
  TEST(PSym_Test);

  TEST(Fail_Test);
  TEST(FXP_Test);
  TEST(ColorMap_Test);
  TEST(Random_Test);
  TEST(BitVector_Test);

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

  TEST(EventWindow_Test);
  TEST(Tile_Test);

  Grid_Test::Test_gridPlaceAtom();

  TEST(ExternalConfig_Test);

  return 0;
}
