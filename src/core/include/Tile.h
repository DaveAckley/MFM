/*                                              -*- mode:C++ -*-
  Tile.h MFM grid segment
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
  \file Tile.h MFM grid segment
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef TILE_H
#define TILE_H

#include <pthread.h>
#include "Dirs.h"
#include "Random.h"  /* for Random */
#include "Packet.h"
#include "Point.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Connection.h"
#include "ThreadPauser.h"
#include "OverflowableCharBufferByteSink.h"  /* for OString16 */

namespace MFM
{
#define IS_OWNED_CONNECTION(X) ((X) - Dirs::EAST >= 0 && (X) - Dirs::EAST < 4)

#define BACKGROUND_RADIATION_SITE_ODDS 1000
#define BACKGROUND_RADIATION_BIT_ODDS 100

  /**
   * An enumeration of the kinds of memory regions which may exist in
   * a Tile.
   */
  typedef enum
  {
    REGION_CACHE   = 0,
    REGION_SHARED  = 1,
    REGION_VISIBLE = 2,
    REGION_HIDDEN  = 3,
    REGION_COUNT
  }TileRegion;

  /**
   * An enumeration of the kinds of locking mechanisms which may exist
   * in a Tile.
   */
  typedef enum
  {
    LOCKTYPE_NONE   = 0,
    LOCKTYPE_SINGLE = 1,
    LOCKTYPE_TRIPLE = 2,
    LOCKTYPE_COUNT
  }LockType;

  /**
   * The representation of a single indefinitely scalable hardware
   * tile (whether real or simulated).
   */
  template <class CC>
  class Tile
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

  public:

    /** The radius of all events taking place in this Tile. */
    static const u32 EVENT_WINDOW_RADIUS = R;

    /** The full length, in sites, of a Tile, including neighbor
        caches.*/
    static const u32 TILE_WIDTH = W;

    /**
     * The area of this Tile in sites.
     */
    static const u32 TILE_SIZE = TILE_WIDTH * TILE_WIDTH;

    static const u32 ELEMENT_TABLE_BITS = B;  // Currently hard-coded!
    static const u32 ELEMENT_TABLE_SIZE = 1u<<ELEMENT_TABLE_BITS;

    /**
     * The edge length of the portion of a Tile that is 'owned' by the
     * Tile itself -- i.e., excluding the cache boundary.
     */
    static const u32 OWNED_SIDE = TILE_WIDTH-2*R;

  private:
    /**
     * A brief name or label for this Tile, for reporting and debugging
     */
    OString16 m_label;

    /** The top of the error stack for thread-local, per-Tile, FAIL
        processing. */
    MFMErrorEnvironmentPointer_t m_errorEnvironmentStackTop;

    /**
     * If \c true , will XRay atoms upon writing.
     */
    bool m_backgroundRadiationEnabled;

    /**
     * If \c true, will not FAIL upon encountering a threading
     * problem. This is usually not recomended, as it causes cache
     * inconsistencies.
     */
    bool m_ignoreThreadingProblems;

#if 0
    /** The 1-in-this odds of bit corruptions during atom writing.  (0
        means no corruptions).  (NOT YET IMPLEMENTED)  */
    u32 m_writeFailureOdds;
#endif

    /** The ElementTable instance which holds all atom behavior for this
        Tile. */
    ElementTable<CC> elementTable;

    /** The PRNG used for generating all random numbers in this Tile. */
    Random m_random;

    /** The number of events executed in this Tile since
        initialization. */
    u64 m_eventsExecuted;

    /** The number of event failures during execution in this Tile
        since initialization. */
    u32 m_eventsFailed;

    /** The number of the event failures resolved by erasing the
        center atom, in this Tile since initialization. */
    u32 m_failuresErased;

    /**
     * The number of events executed in this Tile since
     * initialization. Counts here are indexed by TileRegion, i.e.
     * all hidden events can be read by using
     * m_regionEvents[REGION_HIDDEN] .
     */
    u64 m_regionEvents[REGION_COUNT];

    /**
     * The number of events which have occurred in different locking
     * areas of this Tile.
     */
    u64 m_lockEvents[LOCKTYPE_COUNT];

    // Quick hack counts trying to debug vanishing atoms
    u64 m_lockAttempts;
    u64 m_lockAttemptsSucceeded;

    /**
     * The number of events which have occurred in every individual
     * site. Indexed as m_siteEvents[x][y], x,y : 0..OWNED_SIDE-1.
     */
    u64 m_siteEvents[OWNED_SIDE][OWNED_SIDE];

    /**
     * The event number (m_eventsExecuted) as of the last time the
     * contents of site changed.
     */
    u64 m_lastChangedEventNumber[OWNED_SIDE][OWNED_SIDE];

    friend class EventWindow<CC>;

    /** The Atoms currently held by this Tile, including caches. */
    T m_atoms[TILE_WIDTH][TILE_WIDTH];

    /** An index of the number of each type of Atom currently held
        within this Tile.*/
    s32 m_atomCount[ELEMENT_TABLE_SIZE];

    /** Set to true if an impossible count is detected */
    bool m_needRecount;

    /** A count of corrupted atoms for which an element could not be
        found */
    s32 m_illegalAtomCount;

    /** The local location which the last EventWindow was centered
        on. */
    SPoint m_lastExecutedAtom;

    /** The only EventWindow to exist in this tile. */
    EventWindow<CC> m_executingWindow;

    /** Pointers to Connections to each of this Tile's neighbors,
        indexed by EuclidDir. */
    Connection* m_connections[8];

    /** True if this tile currently holds the lock on the associated
        Connection. */
    bool m_iLocked[8];

    /** True if this tile is currently in FlushAndWaitOnAllBuffers. */
    bool m_isFnWing;
    u32 m_curFnWingDirWaitWord;
    u32 m_origFnWingDirWaitWord;

    /** True if this tile is currently in trying to advance to pause ready. */
    bool m_isA2PRed;

    /** True if this tile is currently calling GetStateBlockingInner for switching. */
    bool m_isGSBIsw;

    /**
     * The real Connections to half of this Tile's neighbors. Indexing
     * begins at EUDIR_EAST and ends at EUDIR_SOUTHWEST .
     */
    Connection m_ownedConnections[4];

    /**
     * The pthread_t which will run when this Tile is started.
     *
     * @sa Start
     */
    pthread_t m_thread;

    /**
     * Tells whether or not m_thread has been initialized. This is used
     * to keep from starting the program with initialized threads.
     */
    bool m_threadInitialized;

    /**
     * Returns true if this Tile is paused, or if the accessing thread
     * is this Tile's owner.
     */
    bool IsPausedOrOwner()
    {
      return m_threadPauser.GetStateNonblocking()==THREADSTATE_PAUSED
        || pthread_equal(pthread_self(), m_thread);
    }

#if 0
    /**
     * Tells whether or not m_thread should be blocking and waiting to
     * be unpaused.
     *
     * @sa Pause
     * @sa Start
     */
    bool m_threadPaused;
#endif

    /**
     * A flag which describes how a Tile should be executing. If \c
     * true , will execute its own events. If \c false , will instead
     * only perform intertile communication, without executing its own
     * events.
     */
    bool m_executeOwnEvents;

    /**
     * The ThreadPauser used to block m_thread when it needs to be paused.
     *
     * @sa Pause
     * @sa Start
     */
    ThreadPauser m_threadPauser;

    /**
     * The generation number of this tile, used to reject obsolete
     * intertile Packets
     */
    u8 m_generation;

    /**
     * Checks to see if this Tile owns the connection over a particular
     * cache.
     *
     * @param dir The cache to check ownership of its connection.
     *
     * @returns true if this Tile owns the Connection over the cache in
     *          the dir direction.
     */
    bool IsOwnedConnection(Dir dir);

    /**
     * Sets m_executingWindow to a new location in this Tile. This new
     * location is guaranteed to be valid and ready for execution.
     */
    void CreateRandomWindow();

    /**
     * Sets m_executingWindow to a specified location in this Tile. After
     * calling, m_executingWindow is ready for execution.
     *
     * @param pt The location to set m_executingWindow to.
     */
    void CreateWindowAt(const SPoint& pt);

    /**
     * Populates m_outgoingPackets with all Packets describing the state of
     * this Tile's caches. All Packets created by this method are ready to
     * be output to other Tiles.
     *
     * @returns a bitfield describing which neighbors need to be
     *          waited on for receipt of an acknowledgement
     *          packet. These neighbors are all found by checking
     *          (retval & (1<<EUDIR_DIRECTION)).
     */
    u32 SendRelevantAtoms();

    /**
     * Alerts a neighboring Tile that this Tile has completed its
     * event. This keeps the inter-tile buffers from overflowing. This
     * should be the last thing called (once per relevant neighbor)
     * during an event.
     *
     * @param neighbor The EuclidDir of the direction of the
     *                 neighboring Tile which should be sent an "Event
     *                 Complete" packet and waited on for an
     *                 acknowledgement Packet.
     */
    void SendEndEventPackets(u32 dirWaitWord);


    /**
     * Compute the coordinates of \c atomLoc in a neighboring tile.
     * (There may or may not actually be a Tile in the given \c
     * neighbor direction; this method doesn't check or care.)
     *
     * @param neighbor The Dir code of the direction of the
     *                 neighboring Tile.
     *
     * @param atomLoc The point to map into the neighboring Tile's
     *                coordinates.
     * @return the mapped coordinate
     */
    static SPoint GetNeighborLoc(Dir neighbor, const SPoint& atomLoc);

    /**
     * Creates a single Packet describing an Atom placement on this
     * Tile's cache, therefore describing an Atom placement on another
     * Tile's main memory. This Packet is placed in m_outgoingPackets,
     * ready to be sent.
     *
     * @param neighbor The EuclidDir direction of the cache which should
     *                 recieve the generated Packet.
     *
     * @param atomLoc  The current location of the Atom which should be
     *                 written to the destination Tile.
     */
    void SendAtom(Dir neighbor, SPoint& atomLoc);

    /**
     * Attempt to lock the specified Connection, if it is in use. This
     * will return immediately regardless of acquiring said lock.
     *
     * @param connectionDir The index of the Connection wanting to lock.
     *
     * @returns true if there is no connection in \c connectionDir.
     *          Otherwise, returns true if the lock for the specified
     *          direction has been acquired, else false.
     */
    bool TryLock(Dir connectionDir);

    /**
     * Attempt to lock the specified Connection, along with the
     * Connections to its left and to its right. This will return
     * immediately regardless of acquiring said locks, and will unlock
     * all regions if any locks cannot be acquired.
     *
     * @param cornerDir The center EuclidDir to target locking of.
     *
     * @returns true if the locks for all of the regions in question
     *          have been acquired.
     *
     * @sa UnlockCorner
     */
    bool TryLockCorner(Dir cornerDir);

    /**
     * Attempt to lock an edge of this Tile. This takes into account the
     * fact that three regions must be locked when attempting to use
     * this Tile's corners.
     *
     * @param regionDir the EuclidDir of the region in which to lock.
     *
     * @returns true if all relevant locks have been acquired.
     *
     * @sa UnlockRegion
     */
    bool LockRegion(Dir regionDir);

    /**
     * Unlocks the Connection of the specified direction, along with the
     * Connections to its left and to its right. This must only be called
     * if these locks have already been acquired!
     *
     * @param corner The center EuclidDir of the three wished to unlock.
     *
     * @sa LockCorner
     */
    void UnlockCorner(Dir corner);

    void UnlockDir(Dir dir) ;

    /**
     * Unlocks an edge of this Tile. This takes into account the fact
     * that three regions must be unlocked when attempting to use this
     * Tile's corners. This must only be called exactly once after a
     * matching call to LockRegion() has returned true!
     *
     * @param regionDir The direction of which to give up the lock.
     *
     * @sa LockRegion
     */
    void UnlockRegion(Dir regionDir);

    /**
     * Gets the TileRegion that a specified index, from the center of
     * the edge of a Tile, reaches to.
     *
     * @param index The index to check region membership of.
     *
     * @returns The TileRegion which index will reach.
     */
    TileRegion RegionFromIndex(const u32 index);

    /**
     * Gets the TileRegion that a specified point is inside of. This
     * method fails upon an argument which does not point at a memory
     * region.
     *
     * @param pt The SPoint, which much be pointing at a memory region
     *           within this Tile, of which to find the region it is
     *           pointing at.
     *
     * @returns The TileRegion that pt is pointing at.
     */
    TileRegion RegionIn(const SPoint& pt);

    /**
     * Checks to see if a specified point is in Hidden memory, and therefore
     * does not require a lock to execute.
     *
     * @param pt The SPoint to check for Hidden memory membership.
     *
     * @returns true if pt is in Hidden memory.
     */
    inline bool IsInHidden(const SPoint& pt);

    /**
     * Performs a single Event on the generated EventWindow .
     */
    void DoEvent(bool locked, Dir lockRegion);

   public:
    void ReportTileStatus(Logger::Level level);

    /**
     * Returns this Tile's label, if any.  May return an empty string,
     * never returns null.
     */
    const char * GetLabel()
    {
      return m_label.GetZString();
    }

    /**
     * Get access to reset or set the Tile's label.
     */
    OString16 & GetLabelPrinter()
    {
      return m_label;
    }

    /**
     * Constructs a new Tile.
     */
    Tile();

    /**
     * Sets a value in this Tile corresponding to its tolerance of
     * threading problems.
     *
     * @param value If \c true , allows this Tile to ignore threading
     *              problems encountered during Tile execution. This
     *              is not normally recommended because since it will
     *              cause cache inconsistencies, but it will most
     *              likely keep your instance of the MFM from crashing
     *              if this is set. If \c false , will crash upon
     *              detecting a threading problem.
     */
    void SetIgnoreThreadingProblems(bool value);

    /**
     * Reinitializes a Tile to a like-new (actually,
     * like-OnceOnlyInit()ed) state.
     */
    void Reinit();

    /**
     * Resets all Atoms and their counts to the Empty atom.
     */
    void ClearAtoms();

    /**
     * Checks part of this Tile's cache assuming the given \c
     * otherTile lies in the given \c direction from this tile, and
     * the two tiles are connected.  Logs debug messages if cache
     * mismatches are found.
     *
     * @remarks This method relies on grid connectivity knowledge that
     *          actual distributed Tiles cannot be expected to
     *          receive.
     */
    void CheckCacheFromDir(Dir direction, const Tile & otherTile);

    /**
     * Sets the tile generation to \c generation
     */
    void SetGeneration(u8 generation)
    {
      m_generation = generation;
    }

    /**
     * Gets the tile generation
     */
    u8 GetGeneration() const
    {
      return m_generation;
    }

    /**
     * Connects another Tile to one of this Tile's caches.
     *
     * @param other The other Tile to connect to this one.
     *
     * @param toCache The cache to share with other.
     */
    void Connect(Tile<CC>& other, Dir toCache);

    /**
     * Finds the Connection which corresponds to one of this tile's
     * caches.
     *
     * @param cache The direction of the held Connection to find.
     *
     * @returns a pointer to the held connection, or NULL if there is
     *          none.
     */
    Connection* GetConnection(Dir cache);

    /**
     * Gets the number of sites in this Tile in sites, excluding caches.
     */
    u32 GetSites() const
    {
      return OWNED_SIDE*OWNED_SIDE;
    }

    /**
     * Gets the width of this Tile in sites, including caches.
     */
    u32 GetTileWidth()
    {
      return TILE_WIDTH;
    }

    /**
     * Gets a reference to this Tile's PRNG.
     * @Returns a reference to this Tile's PRNG.
     */
    Random & GetRandom();

    /**
     * Gets a reference to this Tile's ElementTable.
     *
     * @returns a reference to this Tile's ElementTable.
     */
    ElementTable<CC> & GetElementTable()
    {
      return elementTable;
    }

    /**
     * Gets a const reference to this Tile's ElementTable.
     *
     * @returns a reference to this Tile's ElementTable.
     */
    const ElementTable<CC> & GetElementTable() const
    {
      return elementTable;
    }

    /**
     * Looks up an element type in this Tile's ElementTable.
     *
     * @returns a pointer to the Element associated with \a
     *          elementType, or NULL if there is no such Element.
     */
    const Element<CC> * GetElement(const u32 elementType) const
    {
      return elementTable.Lookup(elementType);
    }

    /**
     * Checks to see if this Tile has another tile in a specified
     * direction.
     *
     * @param dir The direction from this Tile in which to check for a
     *            neighboring Tile.
     *
     * @returns true if this Tile has a neighbor in direction dir, else false.
     */
    inline bool IsConnected(Dir dir) const;

    bool HasAnyConnections(Dir regionDir) const;

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's caches.
     *
     * @param pt The local location which is in question of being inside
     *        of a cache.
     *
     * @returns true if pt is in a cache, else false.
     */
    static inline bool IsInCache(const SPoint& pt);

    /**
     * Checks to see if a specified local point is contained within a
     * Tile's owned sites.
     *
     * @param pt The local location which is in question of being inside
     *        of the Tile's own sites.
     *
     * @returns true if pt is an owned site of this Tile, else false.
     */
    static inline bool IsOwnedSite(const SPoint & location);


    /**
     * Checks to see if a specified local point is contained within a
     * Tile's sites, including caches.  If this method returns true,
     * then the GetAtom methods, applied to this location, will not
     * FAIL.
     *
     * @param pt The local location which is in question of being
     *           inside the bounds of this Tile
     *
     * @returns true if pt is a legal location in a Tile, else false.
     *
     * @sa GetAtom(const SPoint& pt)
     */
    static inline bool IsInTile(const SPoint& pt);

    /**
     * Checks to see if a specified coordinate is contained within a
     * Tile's sites excluding the caches.  Indexing ignores the cache
     * boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).  If this method returns true, then
     * the 'Uncached' methods, applied to this location, will not FAIL.
     *
     * @param pt The coordinate which is in question of being a legal
     *           uncached site index in this Tile
     *
     * @returns true if pt is a legal uncached location in a Tile,
     * else false.
     *
     * @sa GetUncachedAtom(const SPoint& pt)
     * @sa GetUncachedAtom(s32 x, s32 y)
     * @sa GetUncachedSiteEvents(const SPoint site)
     */
    static inline bool IsInUncachedTile(const SPoint& pt);

    /**
     * Checks to see if a specified local point is a site that
     * currently might receive events in this particular Tile.  Note
     * this method is not static, because liveness depends on the
     * connectivity of this Tile.  Therefore, even given the same
     * location as argument, the return value from this method could
     * change over time, if the connectivity of this Tile changes.
     *
     * @param pt The local location which is in question of being
     *           a live site that receives events in this Tile
     *
     * @returns true if pt is a live site in this Tile, else false.
     */
    bool IsLiveSite(const SPoint & location) const;

    /**
     * Gets the number of events executed within this Tile since
     * initialization.
     *
     * @returns The number of events executed within this Tile since
     *          initialization.
     */
    u64 GetEventsExecuted() const
    {
      return m_eventsExecuted;
    }

    /**
     * Checks to see if a specified SPoint is in a given region of this
     * Tile (i.e. cache, shared, visible, or hidden).
     *
     * @param pt The SPoint to check region membership for.
     *
     * @param reach The size, in sites from the edge of the tile, of
     *              this region. For example, a reach of R would
     *              specify searching for Atoms in this Tile's cache.
     *              2*R includes the shared region, and 3*R includes
     *              the visible region.
     *
     * @returns The direction of the cache pointed at by pt, or
     *          (Dir)-1 if pt is not pointing at a cache.
     */
    Dir RegionAt(const SPoint& pt, u32 reach) const;

    /**
     * Finds the cache in this Tile which contains a specified SPoint.
     *
     * @param pt The Point which should specify the location of a cache.
     *
     * @returns The direction of the cache specified by pt, or
     *          (Dir)-1 if there is no such cache.
     */
    Dir CacheAt(const SPoint& pt) const;

    /**
     * Finds the region of shared memory in this Tile which contains a
     * specified SPoint.
     *
     * @param pt The SPoint which should specify a location within
     *           shared memory.
     *
     * @returns The direction of the shared memory specified by pt, or
     *          (Dir)-1 if pt is not in shared memory.
     */
    Dir SharedAt(const SPoint& pt) const;

    /**
     * Finds the region of visible memory in this Tile which contains a
     * specified SPoint.
     *
     * @param pt The SPoint which should specify a location within
     *           visible memory.
     *
     * @returns The direction of the visible memory specified by pt, or
     *          (Dir)-1 if pt is not in visible memory.
     */
    Dir VisibleAt(const SPoint& pt) const;

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetAtom(const SPoint& pt) const
    {
      return GetAtom(pt.GetX(), pt.GetY());
    }

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     *
     * @remarks Because we need to be able to load an Atom's body
     *          through a configuration file, and this is the only
     *          place where Atoms are unique, we need to be able to
     *          access them in a writable way. Therefore, we have this
     *          non-const accessor. Use GetAtom if not writing to this
     *          Atom.
     */
    T* GetWritableAtom(const SPoint& pt)
    {
      return GetWritableAtom(pt.GetX(), pt.GetY());
    }

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param x The x coordinate of the location of the Atom to
     *          retrieve.

     * @param y The y coordinate of the location of the Atom to
     *          retrieve.
     *
     * @returns A pointer to the Atom at the specified location.
     */
    const T* GetAtom(s32 x, s32 y) const
    {
      if (((u32) x) >= TILE_WIDTH || ((u32) y) >= TILE_WIDTH)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
      return &m_atoms[x][y];
    }

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param x The x coordinate of the location of the Atom to
     *          retrieve.

     * @param y The y coordinate of the location of the Atom to
     *          retrieve.
     *
     * @returns A pointer to the Atom at the specified location.
     *
     * @remarks Because we need to be able to load an Atom's body
     *          through a configuration file, and this is the only
     *          place where Atoms are unique, we need to be able to
     *          access them in a writable way. Therefore, we have this
     *          non-const accessor. Use GetAtom if not writing to this
     *          Atom.
     */
    T* GetWritableAtom(s32 x, s32 y)
    {
      if (((u32) x) >= TILE_WIDTH || ((u32) y) >= TILE_WIDTH)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
      return &m_atoms[x][y];
    }

    /**
     * Gets an Atom from a specified point in this Tile.  Indexing
     * ignores the cache boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetUncachedAtom(const SPoint& pt) const
    {
      return GetUncachedAtom(pt.GetX(), pt.GetY());
    }

    /**
     * Gets an Atom from a specified point in this Tile.  Indexing
     * ignores the cache boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param x The x coordinate of the location of the Atom to
     *          retrieve.

     * @param y The y coordinate of the location of the Atom to
     *          retrieve.
     *
     * @returns A pointer to the Atom at the specified location.
     */
    const T* GetUncachedAtom(s32 x, s32 y) const
    {
      return GetAtom(x+R, y+R);
    }


    /**
     * Gets the site event count for a specified point in this Tile.
     * Indexing ignores the cache boundary, so possible range is (0,0)
     * to (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param site The coordinates of the location of the site whose
     *          event count should be retrieved.
     *
     * @param y The y coordinate of the location of the site whose
     *          event count should be retrieved.
     *
     * @returns The total events that have occured at that site
     */
    u64 GetUncachedSiteEvents(const SPoint site) const ;

    /**
     * Gets the 'write age' of a specified point in this Tile.
     * Indexing ignores the cache boundary, so possible range is (0,0)
     * to (OWNED_SIDE-1,OWNED_SIDE-1).  The write age is the number of
     * events on this tilesince the contents of the specified point
     * changed.  To obtain AEPS, divide this value by
     * Tile::GetSites().
     *
     * @param site The coordinates of the location of the site whose
     *          write age should be retrieved.
     *
     * @param y The y coordinate of the location of the site whose
     *          write age should be retrieved.
     *
     * @returns The events since that site's content changed
     */
    u32 GetUncachedWriteAge(const SPoint site) const ;

    /**
     * Given a Packet , sends a complementary acknowledgement Packet
     * describing that this Packet has received and processed all
     * relevant Packets.
     *
     * @param packet The Packet of which a complementary Packet will
     *               be constructed and sent.
     */
    void SendAcknowledgmentPacket(Packet<T>& packet);

    /**
     * Processes a Packet, performing all necessary operations defined by
     * Packet semantics.
     *
     * @param packet The Packet which this Tile will process.
     */
    void ReceivePacket(Packet<T>& packet);

#if 0 /* Doesn't exist? */
    /**
     * Gets a Pointer to the next Packet which is queued inside of
     * m_outgoingPackets, consuming it in the process.
     *
     * @returns A Pointer to the next Packet in m_outgoingPackets, or
     *          NULL if it does not exist.
     */
    Packet<T>* NextPacket();
#endif

    /**
     * Fills a specified Point with the coordinates of the last EventWindow center.
     *
     * @param out The Point to fill with the coordinates of the last
     *            EventWindow center. All previous data held within this
     *            Point will be erased.
     */
    void FillLastExecutedAtom(SPoint& out);

    /**
     * Store an atom anywhere in the Tile, using the 'raw' coordinate
     * system in which (0,0) is in the Tile cache.
     *
     * @param atom The Atom to place within this Tile.
     * @param pt The local location in which to store atom.
     * @sa PlaceInternalAtom
     */
    void PlaceAtom(const T& atom, const SPoint& pt);

    /**
     * Store an atom in the 'locally-owned' portion of a Tile, using the
     * 'natural' coordinate system in which (0,0) is the upper left of
     * the sites that this Tile owns.
     *
     * @param atom The Atom to place within this Tile.
     *
     * @param pt The local location in which to store atom, beginning in
     *           this Tile's main memory.
     *
     * @sa PlaceAtom
     */
    void PlaceInternalAtom(const T& atom, const SPoint& pt)
    {
      PlaceAtom(atom,pt+SPoint(R,R));
    }

    /**
     * Process all incoming and outgoing Packets which are pending on
     * this Tile .
     *
     * @param dirWaitWord A word describing which buffers need to be
     *                    waited on for Packet communication.
     *
     * @return true if any connection locks are currently held (by
     * either end), else false
     */
    bool FlushAndWaitOnAllBuffers(u32 dirWaitWord);

    /**
     * Log warnings if connection buffers are not empty.
     */
    void ReportIfBuffersAreNonEmpty();

    /**
     * Used to tell this Tile whether or not to actually execute any
     * events, or to just wait on any packet communication from other
     * Tiles instead.
     *
     * @param value If value is true, this tells the Tile to begin
     *              executing its own events. Else, this Tile will
     *              only process Packets from other Tiles.
     */
    void SetExecuteOwnEvents(bool value)
    {
      m_executeOwnEvents = value;
    }

    /**
     * Checks to see whether or not this Tile is executing its own
     * events or if it is simply processing other incoming Packets.
     *
     * @returns \c true if this Tile is executing its own events, else
     *          \c false .
     */
    bool GetExecutingOwnEvents() const
    {
      return m_executeOwnEvents;
    }

    /**
     * Executes a single new EventWindow at a randomly chosen location.
     */
    void Execute();

    /**
     * Used during thread execution to call the execution loop.
     *
     * @param tilePtr A pointer to the tile wished to put into
     an execution loop.
    */
    static void* ExecuteThreadHelper(void* tilePtr);

    /**
     * Gets the number of Atoms in this Tile's main memory of a
     * specified type.
     *
     * @param atomType The ElementType of the Atoms to retreive this
     *                 Tile's count of.
     *
     * @returns The number of Atoms in this Tile's main memory of type
     *          atomType.
     */
    u32 GetAtomCount(ElementType atomType) const;

    /**
     * Stores an Atom after ensuring valid indices but performing no
     * other actions (e.g., no counts are adjusted).
     *
     * @param atom An atom to store
     * @param sx x coordinate in Tile (including caches)
     * @param sy y coordinate in Tile (including caches)
     *
     */
    void InternalPutAtom(const T & atom, s32 x, s32 y);

    /**
     * Sets the internal count of Atoms of a specified ElementType.
     *
     * @param atomType The ElementType of the Atoms of which the count
     *                 will change.
     *
     * @param count The new count of all Atoms of type atomType.
     */
    void SetAtomCount(ElementType atomType, s32 count);

    /**
     * Begins event execution on this Tile on its own thread.
     * Events will continue to happen until paused.
     *
     * @sa Pause
     */
    void Start();

    /**
     * Pauses event execution on this Tile. Events will begin
     * again once Start is called.
     *
     * @sa Start
     */
    void Pause();

    /**
     * Sees if this Tile is ready to be run.  We let all tiles respond
     * to a RunRequest before moving on to actually running, so this
     * method tells whether this Tile has seen the run request .
     *
     * @returns \c true if this Tile is ready to be run, else \c
     *          false .
     */
    bool IsRunReady()
    {
      return m_threadPauser.IsRunReady();
    }

    /**
     * Advances this Tile from THREADSTATE_RUN_READY to
     * THREADSTATE_RUNNING.
     */
    void Run()
    {
      m_threadPauser.Run();
    }

    /**
     * Sees if this Tile is ready to be paused. There is some
     * processing to be done between \c PauseRequested() and \c
     * Pause() , so this method tells whether or not it is time to
     * pause each Tile .
     *
     * @returns \c true if this Tile is ready to be paused, else \c
     *          false .
     */
    bool IsPauseReady();

    void RequestPause();

    /**
     * Adds an offset to the count of a particular type of Atom.
     *
     * @param atomType The ElementType of the Atoms of which the count
     *                 will change.
     *
     * @param delta The offset to add to the count of all Atoms of type
     *              atomType.
     */
    void IncrAtomCount(ElementType atomType, s32 delta);

    /** Do a full count of atom types and FAIL unless the atom
        histogram in m_atomCount is consistent with the content of
        m_atoms. */
    void AssertValidAtomCounts() const;

    /**
     * Calls RecountAtoms if anybody has signaled a need to do so.
     * this tile.
     */
    void RecountAtomsIfNeeded();

    /**
     * Resets all atom counts and refreshes the atoms counts inside
     * this tile.
     */
    void RecountAtoms();

    /**
     * Writes a single x-axis raster line of this Tile to a specified
     * ByteSink.
     *
     * @param outstrm The ByteSink to write raster data to.
     *
     */
    u64 WriteEPSRasterLine(ByteSink & outstrm, u32 lineIdx);

    /**
     * Registers an Element into this Tile's ElementTable.
     *
     * @param the Element to register into this Tile's ElementTable.
     */
    void RegisterElement(const Element<CC> & anElement)
    {
      elementTable.RegisterElement(anElement);
    }

    /**
     * Sets whether or not background radiation will begin mutating
     * the Atoms of this Tile upon writing.
     */
    void SetBackgroundRadiation(bool value)
    {
      m_backgroundRadiationEnabled = value;
    }

    /**
     * XRays a single randomly chosen Atom in this Tile, with the odds
     * of flipping a bit being BACKGROUND_RADIATION_BIT_ODDS .
     */
    inline void SingleXRay();

    /**
     * XRays a single Atom in this Tile, with the odds of flipping a
     * bit being BACKGROUND_RADIATION_BIT_ODDS .
     *
     * @param x The x location of the Atom to XRay
     *
     * @param y The y location of the Atom to XRay
     */
    inline void SingleXRay(u32 x, u32 y);

    /**
     * Iterates through each Atom in this Tile, XRaying each atom
     * based on a provided rate and flipping a bit in each selected
     * atom at a provided rate.
     *
     * @param siteOdds The odds of an Atom to be selected for XRay.
     *
     * @param bitOdds The odds that a particular bit in a selected
     *                Atom will be selected for XRay.
     */
    void XRay(u32 siteOdds, u32 bitOdds);

  };
} /* namespace MFM */

#include "Tile.tcc"

#endif /*TILE_H*/
