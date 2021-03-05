/* -*- mode:C++ -*- */
/**
  MFMSTile.h An independent hardware unit capable of tiling space
  Copyright (C) 2014-2018 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2017-2018 Ackleyshack, LLC.  All rights reserved.

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
  \file MFMSTile.h An independent hardware unit capable of tiling space
  \author Trent R. Small.
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2014-2018 All rights reserved.
  \lgpl
 */
#ifndef MFMSTILE_H
#define MFMSTILE_H

#include "Tile.h"

namespace MFM
{
  template <class EC>  // An EventConfig
  class MFMSTile : public Tile<EC>
  {
    typedef Tile<EC> Super;
    typedef typename EC::SITE S;

  public:

    MFMSTile(const u32 tileWidth, const u32 tileHeight, const GridLayoutPattern gridlayout, S * sites, const u32 eventbuffersize, EventHistoryItem * items)
      : Super(tileWidth, tileHeight, gridlayout, sites, eventbuffersize, items)
    { }

    virtual bool IsConnected(Dir dir) const ;
    virtual bool IsCacheSitePossibleEventCenter(const SPoint & location) const;

    virtual ~MFMSTile() ;
  };
} /* namespace MFM */

#include "MFMSTile.tcc"

#endif /*MFMSTILE_H*/
