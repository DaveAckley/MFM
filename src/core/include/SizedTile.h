/*                                              -*- mode:C++ -*-
  SizedTile.h A Tile with size and storage
  Copyright (C) 2015,2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2017 Ackleyshack, LLC.  All rights reserved.

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
  \file SizedTile.h A Tile with size and storage
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2015,2017 All rights reserved.
  \lgpl
 */
#ifndef SIZEDTILE_H
#define SIZEDTILE_H

#include "MFMSTile.h"

namespace MFM
{

  /**
     A SizedTile provides a completed Tile, possessing a size and site
     storage, and offering a default constructor so that arrays of
     SizedTiles can be formed.
   */
  template <class EC, u32 WIDTH, u32 HEIGHT, u32 EVENTHISTORYSIZE>
  class SizedTile : public MFMSTile<EC>
  {
  public:
    typedef typename EC::SITE SITE;

    enum { TILE_WIDTH = WIDTH };
    enum { TILE_HEIGHT = HEIGHT };
    enum { TILE_SITES = TILE_WIDTH * TILE_HEIGHT };

    static void SetGridLayoutPattern(GridLayoutPattern layout){ m_ctorLayoutPattern = layout; }

    static GridLayoutPattern GetGridLayoutPattern(){ return m_ctorLayoutPattern; }

    static bool IsGridLayoutPatternStaggered() { return (m_ctorLayoutPattern == GRID_LAYOUT_STAGGERED); }

    SizedTile(): MFMSTile<EC>(TILE_WIDTH, TILE_HEIGHT, m_ctorLayoutPattern, m_sites, EVENTHISTORYSIZE, m_items) { }


  private:
    SITE m_sites[TILE_SITES];
    EventHistoryItem m_items[EVENTHISTORYSIZE];
    static GridLayoutPattern m_ctorLayoutPattern;

  };

  //define static member for template instances here
  template <class EC, u32 WIDTH, u32 HEIGHT, u32 EVENTHISTORYSIZE>
  GridLayoutPattern SizedTile<EC, WIDTH, HEIGHT, EVENTHISTORYSIZE>::m_ctorLayoutPattern;

} /* namespace MFM */

#endif /*SIZEDTILE_H*/
