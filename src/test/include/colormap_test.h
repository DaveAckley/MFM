#ifndef COLORMAP_TEST_H      /* -*- C++ -*- */
#define COLORMAP_TEST_H

#include "colormap.h"

namespace MFM {

  class ColorMapTest
  {
  private:
    static void Test_colorMapSelected();
    static void Test_colorMapInterpolated();

  public:
    static void Test_RunTests();
  };

} /* namespace MFM */
#endif /*COLORMAP_TEST_H*/

