/*                                              -*- mode:C++ -*-
  PSym.h 2D-Symmetries for the Point structure
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
  \file PSym.h 2D-Symmetries for the Point structure
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PSYM_H
#define PSYM_H

#include "Point.h"
#include "itype.h"

namespace MFM {

  /**
     Names for the eight point symmetries on a 2D rectangular grid --
     four cases for 90 degree anticlockwise rotations of the x axis
     times two cases of the y axis being a left or right turn from the
     x axis.

     A few examples of what we want:
     <pre>
                (1,1)     (1,1)<----(1,0)        (1,1)
                  ^                   ^            ^
                  |                   |            |
                  |                   |            |
      (0,0)---->(1,0)               (0,0)        (1,0)<----(0,0)

       PSYM_DEG000L        PSYM_DEG090L           PSYM_DEG180R
     </pre>

     The 'normal' case we call PSYM_DEG000L -- the x axis rotated 0
     degrees counterclockwise from the typical first quadrant layout,
     and the y axis being a left turn from the x axis.

     (I'm not exactly sure, though, how this all plays with SDL
     rendering.  Does +ive Y go up there?)

   */
  enum PointSymmetry {
    PSYM_DEG000L,      // x -> x, y -> y
    PSYM_DEG090L,      // x -> y, y ->-x
    PSYM_DEG180L,      // x ->-x, y ->-y
    PSYM_DEG270L,      // x ->-y, y -> x
    PSYM_DEG000R,      // x -> x, y ->-y
    PSYM_DEG090R,      // x -> y, y -> x
    PSYM_DEG180R,      // x ->-x, y -> y
    PSYM_DEG270R,      // x ->-y, y ->-x
    PSYM_SYMMETRY_COUNT,
    PSYM_NORMAL=PSYM_DEG000L,
    PSYM_FLIPX=PSYM_DEG180R,
    PSYM_FLIPY=PSYM_DEG000R,
    PSYM_FLIPXY=PSYM_DEG180L
  };

  /**
     A general-purpose point symmetry coord mapping template, for when
     we know at compile-time what transformation we want.
   */
  template <PointSymmetry PSYM>
  inline SPoint Map(const SPoint & in) ;

  template <> inline SPoint Map<PSYM_DEG000L>(const SPoint & in) {  return SPoint( in.GetX(), in.GetY()); }    // x -> x, y -> y
  template <> inline SPoint Map<PSYM_DEG090L>(const SPoint & in) {  return SPoint(-in.GetY(), in.GetX()); }    // x -> y, y ->-x
  template <> inline SPoint Map<PSYM_DEG180L>(const SPoint & in) {  return SPoint(-in.GetX(),-in.GetY()); }    // x ->-x, y ->-y
  template <> inline SPoint Map<PSYM_DEG270L>(const SPoint & in) {  return SPoint( in.GetY(),-in.GetX()); }    // x ->-y, y -> x
  template <> inline SPoint Map<PSYM_DEG000R>(const SPoint & in) {  return SPoint( in.GetX(),-in.GetY()); }    // x -> x, y ->-y
  template <> inline SPoint Map<PSYM_DEG090R>(const SPoint & in) {  return SPoint( in.GetY(), in.GetX()); }    // x -> y, y -> x
  template <> inline SPoint Map<PSYM_DEG180R>(const SPoint & in) {  return SPoint(-in.GetX(), in.GetY()); }    // x ->-x, y -> y
  template <> inline SPoint Map<PSYM_DEG270R>(const SPoint & in) {  return SPoint(-in.GetY(),-in.GetX()); }    // x ->-y, y ->-x

  /**
     A general-purpose point symmetry coord mapping function, for when
     we don't know at compile-time what transformation we want.
   */
  inline SPoint Map(const SPoint & in, const PointSymmetry psym, const SPoint & ifNone) {
    switch (psym) {
    case PSYM_DEG000L: return Map<PSYM_DEG000L>(in);
    case PSYM_DEG090L: return Map<PSYM_DEG090L>(in);
    case PSYM_DEG180L: return Map<PSYM_DEG180L>(in);
    case PSYM_DEG270L: return Map<PSYM_DEG270L>(in);
    case PSYM_DEG000R: return Map<PSYM_DEG000R>(in);
    case PSYM_DEG090R: return Map<PSYM_DEG090R>(in);
    case PSYM_DEG180R: return Map<PSYM_DEG180R>(in);
    case PSYM_DEG270R: return Map<PSYM_DEG270R>(in);
    default: return ifNone;
    }
  }

} /* namespace MFM */

#endif /*PSYM_H*/
