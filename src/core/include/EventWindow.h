/*                                              -*- mode:C++ -*-
  EventWindow.h Main MFM event component
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
  \file EventWindow.h Main MFM event component
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "Point.h"
#include "itype.h"
#include "MDist.h"  /* for EVENT_WINDOW_SITES */
#include "PSym.h"   /* For PointSymmetry, Map */

namespace MFM
{

  // Forward declaration
  template <class CC>
  class Tile;

  /**
   * An EventWindow provides access for an Element to a selected
   * portion of the sites of a Tile to enable the Element::behavior()
   * method to compute a state transition.
   */
  template <class CC>
  class EventWindow
  {
   private:
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

    Tile<CC> & m_tile;

    SPoint m_center;

    PointSymmetry m_sym;

    /**
     * Low-level, private because this does not guarantee loc is in
     * the window!
     */
    SPoint MapToTile(const SPoint & loc) const
    {
      return Map(loc,m_sym,loc)+m_center;
    }

  public:

    /**
     * Checks to see if an SPoint describing a vector relative to the
     * center of this EventWindow, is actually within reach of the
     * center of this EventWindow.
     *
     * @param offset The relative SPoint to check for membership of in
     *               this EventWindow.
     *
     * @returns \c true if \c offset is considered to be inside this
     *          EventWindow.
     */
    bool InWindow(const SPoint & offset) const
    {
      // Ignores m_sym since point symmetries can't change this answer
      return offset.GetManhattanLength() <= R;
    }

    /**
     * FAIL(ILLEGAL_ARGUMENT) if offset is not in the event window
     */
    SPoint MapToTileValid(const SPoint & offset) const ;

    /**
     * Gets the PointSymmetry currently used by this EventWindow .
     *
     * @returns The PointSymmetry currently used by this EventWindow .
     */
    PointSymmetry GetSymmetry() const { return m_sym; }

    /**
     * Sets the PointSymmetry to be used by this EventWindow.
     *
     * @param psym The new PointSymmetry that wil be used by this EventWindow .
     */
    void SetSymmetry(const PointSymmetry psym)
    {
      m_sym = psym;
    }

    /**
     * Gets the Random object used by the Tile that this EventWindow
     * is taking place inside.
     *
     * @returns The PRNG used by the Tile that this EventWindow is
     *          taking place in.
     */
    Random & GetRandom()
    {
      return m_tile.GetRandom();
    }

    /**
     * Gets the Tile that this EventWindow is taking place inside.
     *
     * @returns The Tile that this EventWindow is taking place inside.
     */
    Tile<CC>& GetTile()
    {
      return m_tile;
    }

    /**
     * Checks to see if a particular SPoint, relative to the center of
     * this EventWindow, points to a Site that may be used during
     * event execution.
     *
     * @param location The relative point to check for liveliness in
     *                 this EventWindow .
     *
     * @returns \c true if this site may be reached during event
     *          execution, else \c false .
     */
    bool IsLiveSite(const SPoint & location) const
    {
      return m_tile.IsLiveSite(MapToTile(location));
    }

    /**
     * Constructs a new EventWindow which takes place on a specified
     * Tile with the default PointSymmetry of PSYM_NORMAL .
     *
     * @param tile The Tile which this EventWindow will take place in.
     */
    EventWindow(Tile<CC> & tile) : m_tile(tile), m_sym(PSYM_NORMAL)
    { }

    /**
     * Place this EventWindow within GetTile, in untransformed Tile
     * coordinates.
     *
     * @param center The new center of this EventWindow .
     */
    void SetCenterInTile(const SPoint& center)
    {
      m_center = center;
    }

    /**
     * Get the position this EventWindow within the Tile it resides
     * in, in untransformed Tile coordinates.
     *
     * @returns The center of this EventWindow, relative to the Tile
     *          it is located in.
     */
    const SPoint& GetCenterInTile() const
    {
      return m_center;
    }

    /**
     * Deconstructs this EventWindow.
     */
    ~EventWindow()
    { }

    /**
     * Gets the number of sites that are visible, and therefore
     * mutable, to this EventWindow .
     *
     * @returns The number of active sites in this EventWindow .
     */
    u32 GetAtomCount()
    {
      return EVENT_WINDOW_SITES(R);
    }

    /**
     * Gets the immutable Atom which resides in the center of this
     * EventWindow.
     *
     * @returns The immutable Atom which resides in the center of this
     * EventWindow.
     */
    const T& GetCenterAtom() const
    {
      return *m_tile.GetAtom(m_center);
    }

    /**
     * Sets the Atom in the center of this EventWindow to a specified Atom .
     *
     * @param atom The Atom that will now reside in the center of this
     *             EventWindow .
     */
    void SetCenterAtom(const T& atom)
    {
      return m_tile.PlaceAtom(atom, m_center);
    }

    /**
     * Gets an Atom residing at a specified location inside this EventWindow .
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     */
    const T& GetRelativeAtom(const SPoint& offset) const;

    /**
     * Gets an Atom residing at a specified direction from the center
     * atom inside this EventWindow .
     *
     * @param offset The direction, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     */
    const T& GetRelativeAtom(const Dir mooreOffset) const;

    /**
     * Sets an Atom residing at a specified location in this
     * EventWindow to a specified Atom .
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be set. If this is
     *               not inside the EventWindow, will FAIL with
     *               ILLEGAL_ARGUMENT .
     *
     * @param atom The Atom to place in this EventWindow .
     *
     * @returns \c true .
     */
    bool SetRelativeAtom(const SPoint& offset, const T & atom);

    /**
     * Takes the Atom in a specified location and swaps it with an
     * Atom in another location.
     *
     * @param locA The location of the first Atom to swap
     *
     * @param locB The location of the second Atom to swap
     */
    void SwapAtoms(const SPoint& locA, const SPoint& locB);

    /**
     * Takes the Atom in a specified location and swaps it with an
     * Atom in the center of this EventWindow.
     *
     * @param relative The location of the first Atom to swap with this
     *                 EventWindow's center atom.
     */
    void SwapCenterAtom(const SPoint& relative);

  };
} /* namespace MFM */

#include "EventWindow.tcc"

#endif /*EVENTWINDOW_H*/
