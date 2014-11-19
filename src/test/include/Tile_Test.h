#ifndef TILE_TEST_H      /* -*- C++ -*- */
#define TILE_TEST_H

#include "Test_Common.h"

namespace MFM {

  /**
   * Tests for the Tile class
   */
  class Tile_Test
  {
  public:
    static void Test_RunTests();

    static void Test_tilePlaceAtom();
    static void Test_tileSquareDistances();
  };
} /* namespace MFM */

#endif /*TILE_TEST_H*/
