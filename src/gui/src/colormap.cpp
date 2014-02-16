#include "colormap.h"
#include "fail.h"  /* for FAIL */

namespace MFM {

  static const char *(ColorMapNames[]) = {

#define XX5(T,N,M1,M2,M3,M4,M5)                 \
    ""#T"5_"#N,

#define XX6(T,N,M1,M2,M3,M4,M5,M6)              \
    ""#T"6_"#N,

#include "colormaps.h"

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

#include "colormaps.h"

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

#include "colormaps.h"

#undef XXB
#undef XX5
#undef XX6

  u32 ColorMap::GetSelectedColor(float value, float min, float max, u32 outOfRange) const {
    if (min > max || value < min || value > max) return outOfRange;

    const u32 * map = GetColorArray();
    u32 maxIdx = GetColorArrayLength()-1;

    float range = max-min;
    float frac = (value-min)/range;

    u32 idx = (u32) (frac*maxIdx+0.5);
    if (idx > maxIdx) idx = maxIdx;

    return map[idx];
  }

  u32 ColorMap::GetInterpolatedColor(float value, float min, float max, u32 outOfRange) const {
    if (min > max || value < min || value > max) return outOfRange;

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

