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
#include "Site.h"
#include "Base.h"
#include "ByteSink.h"

namespace MFM
{

  template <class EC> class Tile; // FORWARD

  /**
     An EventWindow provides access for an Element to a selected
     portion of the sites of a Tile to enable the Element::behavior()
     method to compute a state transition.

     The EventWindow <-> Tile interaction during events works as follows:

     - During Tile::AdvanceComputation, tile selects a random coord in
       its owned region, and calls EventWindow::TryEventAt, using its
       (one and only) EventWindow.

     - TryEventAt first does filtering for the desired degree of
       spatial flatness (RejectOnRecency).  Then it goes to
       InitForEvent, which attempts to acquire all necessary locks.
       If either filtering or locking fails, the event attempt fails.
       (If Tile::AdvanceComputation sees a false return from
       TryEventAt, it returns false, indicating the computation did
       not advance.)

     - Otherwise, EventWindow::TryEventAt (via InitForEvent) has
       successfully locked all needed CacheProcessors (if any), and it
       sets up to perform an event at the selected center, loads that
       center's neighborhood from the tile, updates statistics, and
       performs the event, up to the cache notifications stage.  It
       writes all site changes back to its own Tile, in the process
       notifying all locked CacheProcessors about the neighborhood
       contents, and the CacheProcessors copy any atoms they want to
       their shipping buffers.  (CacheProcessors want atoms that are
       (1) visible to the far side and (2) either changed by the
       event, or selected randomly for a redundancy spot check.)

     - At that point, TryEventAt returns, and EventWindow processing
       is finished.  That means Tile::AdvanceComputation is finished,
       and the Tile moves on to AdvanceCommunication, during which it
       advances all its CacheProcessors once.  Then Tile::Advance
       returns.  Subsequent Tile::Advance calls will return to
       EventWindow::TryEventAt, but until all the relevant cache
       processors have finished flushing, the associated directions
       won't be lockable.

   */
  template <class EC>
  class EventWindow
  {
  private:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename EC::SITE S;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };
  public:
    enum { SITE_COUNT = EVENT_WINDOW_SITES(R) };
  private:

    Tile<EC> & m_tile;

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

    Base<AC> m_centerBase;

    SPoint m_center;

    Dir m_lockRegion;

    enum { MAX_CACHES_TO_UPDATE = 3 };
    CacheProcessor<EC> * m_cacheProcessorsLocked[MAX_CACHES_TO_UPDATE];
    RandomIterator<MAX_CACHES_TO_UPDATE> m_cpli;

    PointSymmetry m_sym;

    bool AcquireAllLocks(const SPoint& centerSite) ;

    bool AcquireRegionLocks() ;

    enum LockStatus {
      LOCK_UNNEEDED,
      LOCK_UNAVAILABLE,
      LOCK_ACQUIRED
    };

    LockStatus AcquireDirLock(Dir dir) ;

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
     * Produce the absolute tile location for a given
     * eventwindow-relative coordinate loc.  Maps loc through the
     * current symmetry and then offsets by the event window position.
     * This is a low-level, private method that does not check if
     * loc is actually in the window.
     */
    SPoint MapToTile(const SPoint & loc) const
    {
      return SymMap(loc,m_sym,loc)+m_center;
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

    void ExecuteEvent() ;

    void PrintEventSite(ByteSink & bs) ;

    void ExecuteBehavior() ;

    void InitiateCommunications() ;

    void LoadFromTile() ;

    void StoreToTile() ;

    friend class EventWindow_Test;
    friend class Tile<EC>;

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
    bool TryForceEventAt(const SPoint & center) ;

    bool TryEventAtForTesting(const SPoint & center)
    {
      return TryEventAt(center);
    }

    /**
     * Set up for an event at center, which represented in full,
     * untransformed Tile coordinates.  Public primarily for ulam
     * element testing.
     */
    bool InitForEvent(const SPoint & center) ;

    const Site<AC> & GetSite() const
    {
      return GetTile().GetSite(m_center);
    }

    const Base<AC> & GetBase() const
    {
      return m_centerBase;
    }

    Base<AC> & GetBase()
    {
      return m_centerBase;
    }

    u64 GetEventWindowsAttempted() const
    {
      return m_eventWindowsAttempted;
    }

    u64 GetEventWindowsExecuted() const
    {
      return m_eventWindowsExecuted;
    }

    void SetEventWindowsAttempted(u64 attempts)
    {
      m_eventWindowsAttempted = attempts;
    }

    void SetEventWindowsExecuted(u64 executed)
    {
      m_eventWindowsExecuted = executed;
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
    SPoint MapToTileSymValid(const SPoint & offset) const ;

    /**
     * Map a site number into a point, and then map that point through
     * the current symmetry.  Fails on illegal siteNumbers
     *
     * \sa MapToIndexSymValid
     */
    SPoint MapToPointSymValid(const u32 siteNumber) const
    {
      const MDist<R> & md = MDist<R>::get();
      SPoint direct = md.GetPoint(siteNumber);
      return SymMap(direct,m_sym,direct);
    }

    /**
     * Map a site number into a point, and then map that point through
     * the current symmetry, then map that point back to a site
     * number.  Fails on illegal siteNumbers
     */
    u32 MapIndexToIndexSymValid(const u32 siteNumber) const
    {
      SPoint sym = MapToPointSymValid(siteNumber);
      return MapToIndexDirectValid(sym);
    }

    /**
     * Map a relative coordinate through the psymmetry and into an
     * index into the atomBuffer.  FAIL(ILLEGAL_ARGUMENT) if offset is
     * not in the event window
     */
    u32 MapToIndexSymValid(const SPoint & loc) const ;

    /**
     * Map a relative coordinate into a site number.  Note this method
     * DOES NOT apply the current symmetry to loc.
     * FAIL(ILLEGAL_ARGUMENT) if loc is not in the event window
     *
     * \sa MapToIndexSymValid
     */
    u32 MapToIndexDirectValid(const SPoint & loc) const ;

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
    Tile<EC>& GetTile()
    {
      return m_tile;
    }

    const Tile<EC>& GetTile() const
    {
      return m_tile;
    }

    /**
     * Checks to see if a particular SPoint, relative to the center of
     * this EventWindow, points to a Site that may be used during
     * event execution.  Note this version DOES NOT apply the current
     * symmetry to location
     *
     * @param location The relative point to check for liveness in
     *                 this EventWindow .
     *
     * @returns \c true if this site may be reached during event
     *          execution, else \c false .
     *
     * \sa IsLiveSiteSym
     */
    bool IsLiveSiteDirect(const SPoint & location) const
    {
      return m_isLiveSite[MapToIndexDirectValid(location)];
    }

    /**
     * Checks to see if a particular SPoint, relative to the center of
     * this EventWindow, points to a Site that may be used during
     * event execution, when mapped through the current symmetry.
     *
     * @param location The relative point to check for liveness in
     *                 this EventWindow .
     *
     * @returns \c true if this site may be reached during event
     *          execution, else \c false .
     *
     * \sa IsLiveSite
     */
    bool IsLiveSiteSym(const SPoint & location) const
    {
      return m_isLiveSite[MapToIndexSymValid(location)];
    }

    /**
     * Checks to see if a particular site Number, relative to the
     * center of this EventWindow, points to a Site that may be used
     * during event execution, when mapped through the current
     * symmetry.
     *
     * @param siteNumber The site number to check for liveness in this
     *                 EventWindow .
     *
     * @returns \c true if this site may be reached during event
     *          execution, else \c false .
     *
     * \sa IsLiveSite
     */
    bool IsLiveSiteSym(const u32 siteNumber) const
    {
      return m_isLiveSite[MapIndexToIndexSymValid(siteNumber)];
    }

    /**
     * Constructs a new EventWindow which takes place on a specified
     * Tile with the default PointSymmetry of PSYM_NORMAL .
     *
     * @param tile The Tile which this EventWindow will take place in.
     */
    EventWindow(Tile<EC> & tile) ;

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
     * EventWindow.  Same function as GetCenterAtomSym .
     *
     * @returns The immutable Atom which resides in the center of this
     * EventWindow.
     *
     */
    const T& GetCenterAtomDirect() const
    {
      return m_atomBuffer[0];
    }

    /**
     * Gets a modifiable reference to the Atom at the center of this
     * EventWindow.  Same function as GetCenterAtomSym (since
     * symmetries don't affect the center atom).
     *
     * @returns A reference to the Atom at the center of this
     * EventWindow.
     *
     */
    T& GetCenterAtomDirect() 
    {
      return m_atomBuffer[0];
    }

    /**
     * Gets the immutable Atom which resides in the center of this
     * EventWindow.  Same function as GetCenterAtomDirect  (since
     * symmetries don't affect the center atom).
     *
     * @returns The immutable Atom which resides in the center of this
     * EventWindow.
     */
    const T& GetCenterAtomSym() const
    {
      return m_atomBuffer[0];
    }

    /**
     * Gets a modifiable reference to the Atom at the center of this
     * EventWindow.  Same function as GetCenterAtomDirect (since
     * symmetries don't affect the center atom).
     *
     * @returns A reference to the Atom at the center of this
     * EventWindow.
     *
     */
    T& GetCenterAtomSym()
    {
      return m_atomBuffer[0];
    }

    /**
     * Get a copy of an atom by site number, without mapping
     * siteNumber through the current symmetry
     */
    T GetAtomDirect(u32 siteNumber) const
    {
      MFM_API_ASSERT_ARG(siteNumber < SITE_COUNT);
      return m_atomBuffer[siteNumber];
    }

    /**
     * Get a copy of an atom by site number, after mapping siteNumber
     * through the current symmetry
     */
    T GetAtomSym(u32 siteNumber) const
    {
      return m_atomBuffer[MapIndexToIndexSymValid(siteNumber)];
    }

    /**
     * Write an atom to a given site number, without mapping
     * siteNumber through the current symmetry
     */
    void SetAtomDirect(u32 siteNumber, const T & newAtom)
    {
      MFM_API_ASSERT_ARG(siteNumber < SITE_COUNT);
      m_atomBuffer[siteNumber] = newAtom;
    }

    /**
     * Write an atom after mapping the given siteNumber through the
     * current symmetry
     */
    void SetAtomSym(u32 siteNumber, const T & newAtom)
    {
      m_atomBuffer[MapIndexToIndexSymValid(siteNumber)] = newAtom;
    }

    /**
     * Sets the Atom in the center of this EventWindow to a specified
     * Atom .  This function has the same behavior as SetCenterAtomSym .
     *
     * @param atom The Atom that will now reside in the center of this
     *             EventWindow .
     */
    void SetCenterAtomDirect(const T& atom)
    {
      m_atomBuffer[0] = atom;
    }

    /**
     * Sets the Atom in the center of this EventWindow to a specified
     * Atom .  This function has the same behavior as
     * SetCenterAtomDirect .
     *
     * @param atom The Atom that will now reside in the center of this
     *             EventWindow .
     */
    void SetCenterAtomSym(const T& atom)
    {
      m_atomBuffer[0] = atom;
    }

    /**
     * Gets an Atom residing at a specified location, mapped through
     * the current symmetry.
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     *
     * \sa GetRelativeAtomDirect
     */
    const T& GetRelativeAtomSym(const SPoint& offset) const;

    /**
     * Gets an Atom residing at a specified location, without mapping
     * through the current symmetry.
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     *
     * \sa GetRelativeAtomSym
     */
    const T& GetRelativeAtomDirect(const SPoint& offset) const;

    /**
     * Gets an Atom residing at a specified direction from the center
     * atom inside this EventWindow, mapped through the current
     * symmetry .
     *
     * @param offset The direction, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     *
     * \sa GetRelativeAtomDirect(const Dir)
     */
    const T& GetRelativeAtomSym(const Dir mooreOffset) const;

    /**
     * Gets an Atom residing at a specified direction from the center
     * atom inside this EventWindow, without mapping through the
     * current symmetry .
     *
     * @param offset The direction, relative to the center of this
     *               EventWindow , of the Atom to be retreived. If
     *               this is not inside the EventWindow, will FAIL
     *               with ILLEGAL_ARGUMENT .
     *
     * @returns The Atom at \c offset .
     *
     * \sa GetRelativeAtomSym(const Dir)
     */
    const T& GetRelativeAtomDirect(const Dir mooreOffset) const;

    /**
     * Sets an Atom residing at a specified location, mapped through
     * the current symmetry, to a specified Atom .
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be set. If this is
     *               not inside the EventWindow, will FAIL with
     *               ILLEGAL_ARGUMENT .
     *
     * @param atom The Atom to place in this EventWindow .
     *
     * @returns \c true .
     *
     * \sa SetRelativeAtomDirect
     */
    bool SetRelativeAtomSym(const SPoint& offset, const T & atom);

    /**
     * Sets an Atom residing at a specified location, without mapping
     * through the current symmetry, to a specified Atom .
     *
     * @param offset The location, relative to the center of this
     *               EventWindow , of the Atom to be set. If this is
     *               not inside the EventWindow, will FAIL with
     *               ILLEGAL_ARGUMENT .
     *
     * @param atom The Atom to place in this EventWindow .
     *
     * @returns \c true .
     *
     * \sa SetRelativeAtomSym
     */
    bool SetRelativeAtomDirect(const SPoint& offset, const T & atom);

    /**
     * Takes the Atom in a specified site number and swaps it with an
     * Atom at another site number, with both site numbers mapped
     * through the current symmetry.
     *
     * @param locA The site number of the first Atom to swap
     *
     * @param locB The site number of the second Atom to swap
     */
    void SwapAtomsSym(const u32 siteA, const u32 siteB);

    /**
     * Takes the Atom in a specified site number and swaps it with an
     * Atom at another site number, without mapping either site number
     * through the current symmetry.
     *
     * @param locA The site number of the first Atom to swap
     *
     * @param locB The site number of the second Atom to swap
     */
    void SwapAtomsDirect(const u32 siteA, const u32 siteB);

    /**
     * Takes the Atom in a specified location and swaps it with an
     * Atom in another location, with both locations mapped through
     * the current symmetry.
     *
     * @param locA The location of the first Atom to swap
     *
     * @param locB The location of the second Atom to swap
     */
    void SwapAtomsSym(const SPoint& locA, const SPoint& locB);

    /**
     * Takes the Atom in a specified location and swaps it with an
     * Atom in another location, with neither location mapped through
     * the current symmetry.
     *
     * @param locA The location of the first Atom to swap
     *
     * @param locB The location of the second Atom to swap
     */
    void SwapAtomsDirect(const SPoint& locA, const SPoint& locB);

#if 0
    /**
     * Takes the Atom in a specified location and swaps it with an
     * Atom in the center of this EventWindow.
     *
     * @param relative The location of the first Atom to swap with this
     *                 EventWindow's center atom.
     */
    void SwapCenterAtom(const SPoint& relative);
#endif

  };
} /* namespace MFM */

#include "EventWindow.tcc"

#endif /*EVENTWINDOW_H*/
