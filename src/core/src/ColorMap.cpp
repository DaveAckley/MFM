#include "ColorMap.h"
#include "Fail.h"  /* for FAIL */

namespace MFM {

  static const char *(ColorMapNames[]) = {
    "CubeHelix",
    "CubeHelixRev",

#define XX5(T,N,M1,M2,M3,M4,M5)                 \
    ""#T"5_"#N,

#define XX6(T,N,M1,M2,M3,M4,M5,M6)              \
    ""#T"6_"#N,

#include "ColorMaps.h"

#undef XX5
#undef XX6
  };
  u32 ColorMap::GetMapCount() {
    return sizeof(ColorMapNames)/sizeof(ColorMapNames[0]);
  }

  static ColorMap *(ColorMapInstances[]) = {

#define XX5(T,N,M1,M2,M3,M4,M5)                 \
    &ColorMap_##T##5_##N::THE_INSTANCE,

#define XX6(T,N,M1,M2,M3,M4,M5,M6)              \
    &ColorMap_##T##6_##N::THE_INSTANCE,

    &ColorMap_CubeHelix::THE_INSTANCE,
    &ColorMap_CubeHelixRev::THE_INSTANCE,

#include "ColorMaps.h"

#undef XX5
#undef XX6
  };

  ColorMap & ColorMap::GetMap(u32 index) {
    if (index >= GetMapCount())
      FAIL(ILLEGAL_ARGUMENT);
    return *ColorMapInstances[index];
  }

#define XXB(NAME)                                         \
  const char * ColorMap_##NAME::GetName() const {         \
    return ""#NAME;                                       \
  }                                                       \
  const u32 * ColorMap_##NAME::GetColorArray() const {    \
    return NAME##_ARRAY;                                  \
  }                                                       \
  u32 ColorMap_##NAME::GetColorArrayLength() const {      \
    return sizeof(NAME##_ARRAY)/sizeof(NAME##_ARRAY[0]);  \
  }                                                       \
  ColorMap_##NAME ColorMap_##NAME::THE_INSTANCE;

#define XX5(T,N,M1,M2,M3,M4,M5)                           \
  static u32 T##5_##N##_ARRAY[] = { M1,M2,M3,M4,M5 };     \
  XXB(T##5_##N)

#define XX6(T,N,M1,M2,M3,M4,M5,M6)                        \
  static u32 T##6_##N##_ARRAY[] = { M1,M2,M3,M4,M5,M6 };  \
  XXB(T##6_##N)

static u32 CubeHelix_ARRAY[] = {
  0xff000000,        // 0.000
  0xff0e0410,        // 0.032
  0xff170924,        // 0.065
  0xff1a1237,        // 0.097
  0xff181e48,        // 0.129
  0xff122d54,        // 0.161
  0xff0c3d59,        // 0.194
  0xff064e57,        // 0.226
  0xff055e4e,        // 0.258
  0xff0a6c41,        // 0.290
  0xff157733,        // 0.323
  0xff287e25,        // 0.355
  0xff41811b,        // 0.387
  0xff5f8118,        // 0.419
  0xff7f7d1c,        // 0.452
  0xff9e7929,        // 0.484
  0xffba753f,        // 0.516
  0xffd1725b,        // 0.548
  0xffe0727c,        // 0.581
  0xffe8769e,        // 0.613
  0xffe87ebe,        // 0.645
  0xffe289da,        // 0.677
  0xffd898ef,        // 0.710
  0xffcdaafd,        // 0.742
  0xffc3bbff,        // 0.774
  0xffbccdff,        // 0.806
  0xffbbdcfe,        // 0.839
  0xffc1e8f7,        // 0.871
  0xffccf2f2,        // 0.903
  0xffdbf8f0,        // 0.935
  0xffedfcf4,        // 0.968
  0xffffffff         // 1.000
};
XXB(CubeHelix)

static u32 CubeHelixRev_ARRAY[] = {
  0xffffffff,        // 0.000
  0xffffeffc,        // 0.032
  0xffffe0f2,        // 0.065
  0xffffd3e3,        // 0.097
  0xffffc8ce,        // 0.129
  0xffffbfb6,        // 0.161
  0xffffb99c,        // 0.194
  0xfffcb681,        // 0.226
  0xffebb667,        // 0.258
  0xffd5b751,        // 0.290
  0xffbcba3f,        // 0.323
  0xff9fbd32,        // 0.355
  0xff81bf2c,        // 0.387
  0xff63c12b,        // 0.419
  0xff47c030,        // 0.452
  0xff2dbd3a,        // 0.484
  0xff18b848,        // 0.516
  0xff07af58,        // 0.548
  0xff00a469,        // 0.581
  0xff009579,        // 0.613
  0xff008586,        // 0.645
  0xff007390,        // 0.677
  0xff036094,        // 0.710
  0xff0c4e93,        // 0.742
  0xff173c8c,        // 0.774
  0xff202c7f,        // 0.806
  0xff261e6d,        // 0.839
  0xff291358,        // 0.871
  0xff270a40,        // 0.903
  0xff1f0429,        // 0.935
  0xff120113,        // 0.968
  0xff000000         // 1.000
};
XXB(CubeHelixRev)

#include "ColorMaps.h"

#undef XXB
#undef XX5
#undef XX6

  u32 ColorMap::GetSelectedColor(float value, float min, float max, u32 outOfRange) const {
    if (min >= max || value < min || value > max) return outOfRange;

    const u32 * map = GetColorArray();
    u32 maxIdx = GetColorArrayLength()-1;

    float range = max-min;
    float frac = (value-min)/range;

    u32 idx = (u32) (frac*maxIdx+0.5);
    if (idx > maxIdx) idx = maxIdx;

    return map[idx];
  }

  u32 ColorMap::GetInterpolatedColor(float value, float min, float max, u32 outOfRange) const {
    if (min >= max || value < min || value > max) return outOfRange;

    const u32 * map = GetColorArray();
    u32 maxIdx = GetColorArrayLength()-1;

    float range = max-min;
    float frac = (value-min)/range*maxIdx; // portion of full index range

    u32 idx1 = (u32) frac;
    if (idx1==maxIdx)
      return map[maxIdx];

    u32 idx2 = idx1 + 1;
    frac -= idx1;               // portion from idx1 to idx2

    u32 c1 = map[idx1];
    u32 c2 = map[idx2];
    u32 res = 0;
    for (int i = 0; i < 4; ++i) {  // We interpolate the alpha too, just in case..
      s32 comp1 = (c1>>(i<<3))&0xff;
      s32 comp2 = (c2>>(i<<3))&0xff;

      s32 inter = (comp2-comp1)*frac+comp1+0.5;
      res |= (inter&0xff)<<(i<<3);
    }
    return res;
  }

} /* namespace MFM */

