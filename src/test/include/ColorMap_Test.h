#ifndef COLORMAP_TEST_H      /* -*- C++ -*- */
#define COLORMAP_TEST_H

#include "ColorMap.h"

namespace MFM {

  /**
   * Tests of ColorMap
   */
  class ColorMap_Test
  {
  private:
    static void Test_colorMapSelected();
    static void Test_colorMapInterpolated();

  public:
    static void Test_RunTests();
  };

} /* namespace MFM */
#endif /*COLORMAP_TEST_H*/
