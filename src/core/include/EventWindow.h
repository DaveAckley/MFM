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
#include "CacheProcessor.h"
#include "MDist.h"  /* for EVENT_WINDOW_SITES */
#include "PSym.h"   /* For PointSymmetry, Map */

namespace MFM
{

  // Forward declaration
  template <class CC>
  class Tile;

  /**
     An EventWindow provides access for an Element to a selected
     portion of the sites of a Tile to enable the Element::behavior()
     method to compute a state transition.

     The EventWindow <-> Tile interaction during events works as follows:

     - During Tile::AdvanceComputation, tile selects an available
       EventWindow (one in state FREE).  If there are none, the
       computation cannot be advanced.

     - Tile calls EventWindow::TryEvent on the selected EventWindow.

     - TryEvent selects a random tile location and attempts to perform
       an event there.  If it can't acquire all locks, it sets itself
       FREE and returns false.  (If Tile::AdvanceComputation sees a
       false return from TryEvent, it returns false, indicating the
       computation did not advance.)

     - Otherwise, TryEvent has successfully acquired all needed locks
       (if any), and it performs an event at the selected center, up
       to the cache notifications stage.  It writes all site changes
       back to its own Tile, in the process creating a bitmask
       indicating which sites have changed, enters state COMMUNICATE,
       and returns the result of a call on EventWindow::Advance().

     - On EventWindow::Advance() in state COMMUNICATE, the EventWindow
       queues outbound site update packets to each locked neighbor,
       using the bitmask and a per-neighbor counter to track which
       update needs to be sent next, if buffer full conditions prevent
       the entire update from being sent simultaneously (can this
       happen?  Can we engineer it out?)  It may also send redundant
       'check packets' to spot check the neighbor's cache for
       consistency.  On each call to Advance(), the EventWindow
       handles as many inbound and outbound packets as it can without
       blocking, and returns true until everything is completed and
       the EventWindow has returned to state FREE, at which point
       Advance returns false.

     - While in state COMMUNICATE, the EventWind
   */
  template <class CC>
  class EventWindow
  {
   private:
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { SITE_COUNT = EVENT_WINDOW_SITES(R) };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

    Tile<CC> & m_tile;

    u64 m_eventWindowsAttempted;
    u64 m_eventWindowsExecuted;

    void RecordEventAtTileCoord(const SPoint tcoord) ;

    /**
     * Note the atom buffer is maintained in 'direct' coordinates, as
     * if the chosen symmetry is always PSYM_NORMAL.  For accesses by
     * Elements, the current symmetry must be applied to coordinates
     * before accessing the buffer.
     */
    T m_atomBuffer[SITE_COUNT];

    bool m_isLiveSite[SITE_COUNT];

    SPoint m_center;

    Dir m_lockRegion;

    enum { MAX_CACHES_TO_UPDATE = 3 };
    CacheProcessor<CC> * m_cacheProcessorsLocked[MAX_CACHES_TO_UPDATE];

    PointSymmetry m_sym;

    bool AcquireAllLocks(const SPoint& centerSite) ;

    bool AcquireRegionLocks() ;

    /**
       EventWindow states
     */
    enum State
    {
      FREE,
      COMPUTE,
      COMMUNICATE
    };
    State m_ewState;

    /**
     * Low-level, private because this does not guarantee loc is in
     * the window!
     */
    SPoint MapToTile(const SPoint & loc) const
    {
      return Map(loc,m_sym,loc)+m_center;
    }

    /**
     * Map a relative coordinate through the psymmetry and into an
     * index into the atomBuffer.  Returns -1 for illegal coords
     */
    s32 MapToIndex(const SPoint & loc) const
    {
      const MDist<R> & md = MDist<R>::get();
      return FromPoint(Map(loc,m_sym,loc),R);
    }

    /**
     * Attempt to start an event at location center (represented in
     * full, untransformed coordinates inside this
     * EventWindow::GetTile()), returning true if all necessary locks
     * were acquired (and therefore all such locks are now held), and
     * false if any lock was not acquired (and therefore no locks are
     * now held).
     *
     * If TryEventAt returns true, caller must then call Advance()
     * from time to time until Advance() returns false.
     *
     * @param center The center of this EventWindow .
     *
     * \sa Advance
     */
    bool TryEventAt(const SPoint & center) ;

    bool RejectOnRecency(const SPoint tcoord) ;

    /**
     * Set up for an event at center, which represented in full,
     * untransformed Tile coordinates.
     */
    bool InitForEvent(const SPoint & center) ;

    void ExecuteEvent() ;

    void ExecuteBehavior() ;

    void InitiateCommunications() ;

    void LoadFromTile() ;

    void StoreToTile() ;

    friend class EventWindow_Test;
    friend class Tile<CC>;

    /**
     * Attempt to lock the specified direction for use by this
     * EventWindow, without waiting.
     *
     * @param connectionDir The direction of the Channel to lock.
     *
     * @returns true if there is no connection in \c connectionDir.
     *          Otherwise, returns true if the lock for the specified
     *          direction has been acquired, else false.
     */
    bool TryLockDir(Dir connectionDir);

    /**
     * Unlock the specified direction.  Returns silently if the
     * relevant ChannelEnd does not have a Channel.  Otherwise, if the
     * underlying Channel is locked by our side, unlock it and return
     * silently.  Otherwise fail.
     *
     * @param connectionDir The index of the ChannelEnd wanted to be locked.
     *
     * @returns true if there is no connection in \c connectionDir.
     *          Otherwise, returns true if the lock for the specified
     *          direction has been acquired, else false.
     *
     * @fails LOCK_FAILURE if the Channel is not currently locked by
     * us.
     */
    void UnlockDir(Dir connectionDir);


  public:
    u64 GetEventWindowsAttempted() const
    {
      return m_eventWindowsAttempted;
    }

    u64 GetEventWindowsExecuted() const
    {
      return m_eventWindowsExecuted;
    }

    void Diffuse() ;

    bool IsFree() const
    {
      return m_ewState == FREE;
    }

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
       Does any I/O that's appropriate to the situation and possible.

       @returns \c true while internal event operations are still in
       progress. Returns \c false when all internal event operations
       are complete and a new call to MaybeStartEventAt() could be
       made.
     */
    bool Advance() ;

    /**
     * FAIL(ILLEGAL_ARGUMENT) if offset is not in the event window
     */
    SPoint MapToTileValid(const SPoint & offset) const ;

    /**
     * Map a relative coordinate through the psymmetry and into an
     * index into the atomBuffer.  FAIL(ILLEGAL_ARGUMENT) if offset is
     * not in the event window
     */
    u32 MapToIndexValid(const SPoint & loc) const ;

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
      return GetTile().GetRandom();
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
      return m_isLiveSite[MapToIndexValid(location)];
    }

    /**
     * Constructs a new EventWindow which takes place on a specified
     * Tile with the default PointSymmetry of PSYM_NORMAL .
     *
     * @param tile The Tile which this EventWindow will take place in.
     */
    EventWindow(Tile<CC> & tile) ;

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
      return m_atomBuffer[0];
    }

    /**
     * Sets the Atom in the center of this EventWindow to a specified Atom .
     *
     * @param atom The Atom that will now reside in the center of this
     *             EventWindow .
     */
    void SetCenterAtom(const T& atom)
    {
      m_atomBuffer[0] = atom;
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
