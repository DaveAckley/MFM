/*                                              -*- mode:C++ -*-
  ColorMap.h Color mapping system
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file ColorMap.h Color mapping system
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef COLORMAP_H      /* -*- C++ -*- */
#define COLORMAP_H

#include "itype.h"

namespace MFM
{
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
  class ColorMap
  {
   public:
    /**
     * Map value, which must be in the range of min..max to one of
     * five different colors.  If max<=min or value is out of range,
     * return badColor.  Only five or six different colors (plus
     * possibly badColor) will be returned by this function.
     */
    u32 GetSelectedColor(float value, float min, float max, u32 badColor) const;

    /**
     * Map value, which must be in the range of min..max to a color
     * interpolated linearly between five or six different colors
     * equally spaced from min to max.  If max<=min or value is out of
     * range, return badColor.
     */
    u32 GetInterpolatedColor(float value, float min, float max, u32 badColor) const;

    /**
     * Gets the number of ColorMaps loaded into the system.
     *
     * @returns The number of ColorMaps loaded into the system.
     */
    static u32 GetMapCount();

    /**
     * Gets the ColorMap loaded into the system at a specified index.
     *
     * @param index The index of the ColorMap to retrieve.
     *
     * @returns The ColorMap loaded into the system at \c index .
     */
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

// A Dave Green 'cubehelix' map
// # Tue Jul 29 2014 05:04:08 GMT-0600 (MDT)
// # ---------------------------------------------
// # HEX R/G/B cubehelix colour scheme
// #
// # see http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/
// #----------------------------------------------
// # see Green (2011), BASI, 39, 289.
// #
// # start............: 0.5
// # rotations........: -1.5
// # hue..............: 1.4
// # gamma............: 1.0
// # number of levels.: 32
// #----------------------------------------------
// # Dave Green: dag@mrao.cam.ac.uk
// #----------------------------------------------
XXB(CubeHelix)

// # Tue Jul 29 2014 05:36:46 GMT-0600 (MDT)
// # ---------------------------------------------
// # HEX R/G/B cubehelix colour scheme
// #
// # see http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/
// #----------------------------------------------
// # see Green (2011), BASI, 39, 289.
// #
// # start............: 0.5
// # rotations........: -1
// # hue..............: 2.0
// # gamma............: 1.0
// # number of levels.: 32
// # (flipped white to black)
// #----------------------------------------------
// # Dave Green: dag@mrao.cam.ac.uk
// #----------------------------------------------

XXB(CubeHelixRev)

#include "ColorMaps.h"

#undef XXB
#undef XX5
#undef XX6

} /* namespace MFM */

#endif /*COLORMAP_H*/
