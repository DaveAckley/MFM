#ifndef COLORMAP_H      /* -*- C++ -*- */
#define COLORMAP_H

/**
 * A bunch of color scales and gradients for heat-map and similar
 * types of displays.  Based on suggestions from colorbrewer2.org,
 * restricting to 'colorblind safe' and 'print friendly' choices.  We
 * include samples of 'sequential', 'diverging', and 'qualitative'
 * palettes (see colorbrewer2.org for details), and both sets of five
 * colors, which have a distinguished center color, and sets of six
 * colors, which have a distinguished center 'break' between two
 * colors
 */

#include "itype.h"

namespace MFM {

  class ColorMap
  {
  public:

    /**
     * Map value, which must be in the range of min..max to one of
     * five different colors.  If max<min or value is out of range,
     * return badColor.  Only five or six different colors (plus
     * possibly badColor) will be returned by this function.
     */
    u32 GetSelectedColor(float value, float min, float max, u32 badColor) const;

    /**
     * Map value, which must be in the range of min..max to a color
     * interpolated linearly between five or six different colors
     * equally spaced from min to max.  If max<min or value is out of
     * range, return badColor.
     */
    u32 GetInterpolatedColor(float value, float min, float max, u32 badColor) const;

    static u32 GetMapCount();
    static ColorMap & GetMap(u32 index);

  private:
    virtual const u32 * GetColorArray() const = 0;
    virtual const char * GetName() const = 0;
    virtual u32 GetColorArrayLength() const = 0;
  };


#define XXB(NAME)                                     \
  /** ColorMap_##NAME colormap                        \
   */                                                 \
  class ColorMap_##NAME : public ColorMap {           \
    virtual const char * GetName() const ;            \
    virtual const u32 * GetColorArray() const ;       \
    virtual u32 GetColorArrayLength() const ;         \
  public:                                             \
     static ColorMap_##NAME THE_INSTANCE;             \
  };
#define XX5(T,N,M1,M2,M3,M4,M5)                       \
  XXB(T##5_##N)
#define XX6(T,N,M1,M2,M3,M4,M5,M6)                    \
  XXB(T##6_##N)

#include "colormaps.h"

#undef XXB
#undef XX5
#undef XX6

} /* namespace MFM */

#endif /*COLORMAP_H*/

