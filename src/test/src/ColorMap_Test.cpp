#include "assert.h"
#include "ColorMap_Test.h"
#include "itype.h"

namespace MFM {

  static void assertColorsClose(u32 c1, u32 c2) {
    for (u32 i = 0; i < 4; ++i) {
      s32 comp1 = (c1 >> (i << 3)) & 0xff;
      s32 comp2 = (c2 >> (i << 3)) & 0xff;

      assert(comp1 - comp2 < 2 && comp2 - comp1 < 2);
    }
  }

  void ColorMap_Test::Test_RunTests() {
    Test_colorMapSelected();
    Test_colorMapInterpolated();
  }

  void ColorMap_Test::Test_colorMapSelected()
  {
    for (u32 i = 0; i < 5; ++i) {
      u32 frac = i*255/4;  // five even steps from black to white..
      u32 color = 0xff000000|(frac<<16)|(frac<<8)|(frac);
      assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i,0,4,0xffff0000)==color);

      assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i*10,0,4*10,0xffff0000)==color);
      if (i!=0) {
        assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i*10-1,0,4*10,0xffff0000)==color);
        assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i*10-2,0,4*10,0xffff0000)==color);
      }
      if (i!=4) {
        assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i*10+1,0,4*10,0xffff0000)==color);
        assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetSelectedColor(i*10+2,0,4*10,0xffff0000)==color);
      }
    }

  }

  void ColorMap_Test::Test_colorMapInterpolated()
  {
    // Half scale should be 50% grey on 5 or 6 scales, forward or backwards
    assertColorsClose(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(0.5,0,1,0xffff0000),0xff808080);
    assertColorsClose(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(0.5,0,1,0xffff0000),0xff808080);
    assertColorsClose(ColorMap_DBG5_WHBK::THE_INSTANCE.GetInterpolatedColor(0.5,0,1,0xffff0000),0xff808080);
    assertColorsClose(ColorMap_DBG6_WHBK::THE_INSTANCE.GetInterpolatedColor(0.5,0,1,0xffff0000),0xff808080);

    // 0.0 should be black on either scale, and rev for rev scale
    assertColorsClose(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(0.0,0,1,0xffff0000),0xff000000);
    assertColorsClose(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(0.0,0,1,0xffff0000),0xff000000);
    assertColorsClose(ColorMap_DBG5_WHBK::THE_INSTANCE.GetInterpolatedColor(1.0,0,1,0xffff0000),0xff000000);
    assertColorsClose(ColorMap_DBG6_WHBK::THE_INSTANCE.GetInterpolatedColor(1.0,0,1,0xffff0000),0xff000000);

    // 1.0 should be white on either scale, and rev for rev scale
    assertColorsClose(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(1.0,0,1,0xffff0000),0xffffffff);
    assertColorsClose(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(1.0,0,1,0xffff0000),0xffffffff);
    assertColorsClose(ColorMap_DBG5_WHBK::THE_INSTANCE.GetInterpolatedColor(0.0,0,1,0xffff0000),0xffffffff);
    assertColorsClose(ColorMap_DBG6_WHBK::THE_INSTANCE.GetInterpolatedColor(0.0,0,1,0xffff0000),0xffffffff);

    // Out of range values should be caught
    assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(1.00001,0,1,0xffff0000)==0xffff0000);
    assert(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(-0.0001,0,1,0xffff0000)==0xffff0000);

    // Other equivalent scales should be okay
    assertColorsClose(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(5,0,10,0xffff0000),0xff808080);
    assertColorsClose(ColorMap_DBG5_WHBK::THE_INSTANCE.GetInterpolatedColor(-100,-350,150,0xffff0000),0xff808080);
    assertColorsClose(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(1,1,3,0xffff0000),0xff000000);
    assertColorsClose(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(0.0,0,1000,0xffff0000),0xff000000);
    assertColorsClose(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(-2.0,-12,-2,0xffff0000),0xffffffff);
    assertColorsClose(ColorMap_DBG5_WHBK::THE_INSTANCE.GetInterpolatedColor(9,8.9,9,0xffff0000),0xff000000);

    assert(ColorMap_DBG5_BKWH::THE_INSTANCE.GetInterpolatedColor(10.0001,0,10,0xffff0000)==0xffff0000);
    assert(ColorMap_DBG6_BKWH::THE_INSTANCE.GetInterpolatedColor(-10.0001,-10,1,0xffff0000)==0xffff0000);

    // Check red-blue ramp
    for (u32 pct = 0; pct <= 100; ++pct) {
      float frac = 1.0*pct/100;
      u32 rise = 255*frac;
      u32 fall = 255*(1-frac);
      u32 redToBlue = 0xff000000|(fall<<16)|(rise<<0);
      assertColorsClose(ColorMap_DBG5_RDBU::THE_INSTANCE.GetInterpolatedColor(pct,0,100,0xffff0000),redToBlue);
      assertColorsClose(ColorMap_DBG6_RDBU::THE_INSTANCE.GetInterpolatedColor(pct,0,100,0xffff0000),redToBlue);
    }
  }

} /* namespace MFM */
