/*                                              -*- mode:C++ -*-
  ElementUtils.h Element-related helper methods
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
  \file ElementUtils.h Element-related helper methods
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENTUTILS_H
#define ELEMENTUTILS_H

#include "Element.h"
#include "EventWindow.h"

namespace MFM
{
  namespace ElementUtils
  {
    extern const SPoint VNNeighbors[4];

   /**
     * Fills a specified SPoint with a randomly selected Von Neumann
     * neighbor of the center of a specified EventWindow .
     *
     * @param window The EventWindow whose center will be scanned for
     *               an empty Von Neumann neighbor.
     *
     * @param pt The SPoint to fill a Von Neumann unit vector with.
     *
     * @returns \c true if an empty Von Neumann neighbor is found,
     *          else \c false .
     */
    template <class CC>
    bool FillAvailableVNNeighbor(EventWindow<CC>& window, SPoint& pt);

    /**
     * Master search method for finding an Atom inside an EventWindow .
     *
     * @param window The EventWindow to search.
     *
     * @param pt Output, representing a valid location in this search
     *           which satisfies the search parameters.
     *
     * @param relevants The range of SPoints which should be searched
     *                  using this method.
     *
     * @param relevantCount The number of SPoints held by \c relevants .
     *
     * @param rotation A symmetric rotation, describing the flipping
     *                 of the relevant points. This can be used to
     *                 search for a particular corner, then flipped to
     *                 searchfor another.
     *
     * @param type The type of the Element to search for during this
     *             search.
     *
     *
     * @returns \c true if any Atoms are found which satisfy the
     *          search parameters, else \c false .
     */
    template <class CC>
    bool FillPointWithType(EventWindow<CC>& window,
                           SPoint& pt, const SPoint* relevants, u32 relevantCount,
                           Dir rotation, ElementType type);
  } /* namespace ElementUtils */
} /* namespace MFM */

#include "ElementUtils.tcc"

#endif /* ELEMENTUTILS.H */
