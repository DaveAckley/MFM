#ifndef TILE_H      /* -*- C++ -*- */
#define TILE_H

#include <pthread.h>
#include "eucliddir.h"
#include "random.h"  /* for Random */
#include "packet.h"
#include "packetbuffer.h"
#include "point.h"
#include "eventwindow.h"
#include "elementtable.h"
#include "Connection.h"
#include "ThreadPauser.h"


namespace MFM {

  /** The full length, in sites, of a Tile, including neighbor
      caches.*/
#define TILE_WIDTH 40

  /** The number of sites a Tile contains. */
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)


#define IS_OWNED_CONNECTION(X) ((X) - EUDIR_EAST >= 0 && (X) - EUDIR_EAST < 4)

  typedef enum
  {
    REGION_CACHE   = 0,
    REGION_SHARED  = 1,
    REGION_VISIBLE = 2,
    REGION_HIDDEN  = 3,
    REGION_COUNT
  }TileRegion;

  typedef enum
  {
    LOCKTYPE_NONE   = 0,
    LOCKTYPE_SINGLE = 1,
    LOCKTYPE_TRIPLE = 2,
    LOCKTYPE_COUNT
  }LockType;

  template <class T,u32 R>
  class Tile
  {
  public:
    /** The radius of all events taking place in this Tile. */
    static const u32 EVENT_WINDOW_RADIUS = R;

    static const u32 ELEMENT_TABLE_BITS = 8;  // Currently hard-coded!
    static const u32 ELEMENT_TABLE_SIZE = 1u<<ELEMENT_TABLE_BITS;

    /**
     * The edge length of the portion of a Tile that is 'owned' by the
     * Tile itself -- i.e., excluding the cache boundary.
     */
    static const u32 OWNED_SIDE = TILE_WIDTH-2*R;

  private:

    /** The ElementTable instance which holds all atom behavior for this
	Tile. */
    ElementTable<T,R,ELEMENT_TABLE_BITS> elementTable;

    /** The PRNG used for generating all random numbers in this Tile. */
    Random m_random;

    /** The number of events executed in this Tile since
	initialization. */
    u64 m_eventsExecuted;

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

    friend class EventWindow<T,R>;

    /** The Atoms currently held by this Tile, including caches. */
    T m_atoms[TILE_SIZE];

    /** An index of the number of each type of Atom currently held
	within this Tile.*/
    s32 m_atomCount[ELEMENT_TABLE_SIZE];

    /** The local location which the last EventWindow was centered
	on. */
    SPoint m_lastExecutedAtom;

    /** The only EventWindow to exist in this tile. */
    EventWindow<T,R> m_executingWindow;

    /** Pointers to Connections to each of this Tile's neighbors,
	indexed by EuclidDir. */
    Connection* m_connections[8];

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
     * Tells whether or not m_thread should be blocking and waiting to
     * be unpaused.
     *
     * @sa Pause
     * @sa Start
     */
    bool m_threadPaused;

    /**
     * The ThreadPauser used to block m_thread when it needs to be paused.
     *
     * @sa Pause
     * @sa Start
     */
    ThreadPauser m_threadPauser;

    /**
     * Checks to see if this Tile owns the connection over a particular
     * cache.
     *
     * @param dir The cache to check ownership of its connection.
     *
     * @returns true if this Tile owns the Connection over the cache in
     *          the dir direction.
     */
    bool IsOwnedConnection(EuclidDir dir);

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
    void SendAtom(EuclidDir neighbor, SPoint& atomLoc);

    /**
     * Attempt to lock only the Connection specified. This will return
     * immediately regardless of acquiring said lock.
     *
     * @param connectionDir The index of the Connection wanting to lock.
     *
     * @returns true if the lock for the specified direction is
     *          acquired.
     */
    bool TryLock(EuclidDir connectionDir);

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
    bool TryLockCorner(EuclidDir cornerDir);

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
    bool LockRegion(EuclidDir regionDir);

    /**
     * Unlocks the Connection of the specified direction, along with the
     * Connections to its left and to its right. This must only be called
     * if these locks have already been acquired!
     *
     * @param corner The center EuclidDir of the three wished to unlock.
     *
     * @sa LockCorner
     */
    void UnlockCorner(EuclidDir corner);

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
    void UnlockRegion(EuclidDir regionDir);

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

  public:

    /**
     * Constructs a new Tile.
     */
    Tile();

    /**
     * Connects another Tile to one of this Tile's caches.
     *
     * @param other The other Tile to connect to this one.
     *
     * @param toCache The cache to share with other.
     */
    void Connect(Tile<T,R>& other, EuclidDir toCache);

    /**
     * Finds the Connection which corresponds to one of this tile's
     * caches.
     *
     * @param cache The direction of the held Connection to find.
     *
     * @returns a pointer to the held connection, or NULL if there is
     *          none.
     */
    Connection* GetConnection(EuclidDir cache);

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
    ElementTable<T,R,ELEMENT_TABLE_BITS> & GetElementTable() {
      return elementTable;
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
    inline bool IsConnected(EuclidDir dir);

    /**
     * Checks to see if a specified local point is contained within one
     * of this Tile's caches.
     *
     * @param pt The local location which is in question of being inside
     *        of a cache.
     *
     * @returns true if pt is in a cache, else false.
     */
    static inline bool IsInCache(const SPoint& pt);

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
     *              this region. For example, a reach of R would specify
     *              searching for Atoms in this Tile's cache.
     *
     * @returns The EuclidDir of the cache pointed at by pt, or
     *          (EuclidDir)-1 if pt is not pointing at a cache.
     */
    EuclidDir RegionAt(const SPoint& pt, u32 reach);

    /**
     * Finds the cache in this Tile which contains a specified SPoint.
     *
     * @param pt The Point which should specify the location of a cache.
     *
     * @returns The direction of the cache specified by pt, or
     *          (EuclidDir)-1 if there is no such cache.
     */
    EuclidDir CacheAt(const SPoint& pt);

    /**
     * Finds the region of shared memory in this Tile which contains a
     * specified SPoint.
     *
     * @param pt The SPoint which should specify a location within
     *           shared memory.
     *
     * @returns The direction of the shared memory specified by pt, or
     *          (EuclidDir)-1 if pt is not in shared memory.
     */
    EuclidDir SharedAt(const SPoint& pt);


    /*
     * Expose the underlying atom array because, e.g., testing depends
     * on it?  Urgh?
     *
     * Apparently testing no longer depends on this. Seal it up!
     *
     T* GetAtoms() ;
    */

    /**
     * Gets an Atom from a specified point in this Tile.
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetAtom(const SPoint& pt) const;

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
    const T* GetAtom(s32 x, s32 y) const;

    /**
     * Gets an Atom from a specified point in this Tile.  Indexing
     * ignores the cache boundary, so possible range is (0,0) to
     * (OWNED_SIDE-1,OWNED_SIDE-1).
     *
     * @param pt The location of the Atom to retrieve.
     *
     * @returns A pointer to the Atom at location pt.
     */
    const T* GetUncachedAtom(const SPoint& pt) const;

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
      const T* GetUncachedAtom(s32 x, s32 y) const;

    /**
     * Gets an Atom from this Tile by its raster index.
     *
     * @param i The raster index (horizontal first) of the Atom to
     *          retrieve.
     *
     * @returns A pointer to the Atom at the specified index.
     */
    const T* GetAtom(int i) const;

    void SendAcknowledgmentPacket(Packet<T>& packet);

    /**
     * Processes a Packet, performing all necessary operations defined by
     * Packet symantics.
     *
     * @param packet The Packet which this Tile will process.
     */
    void ReceivePacket(Packet<T>& packet);

    /**
     * Gets a Pointer to the next Packet which is queued inside of
     * m_outgoingPackets, consuming it in the process.
     *
     * @returns A Pointer to the next Packet in m_outgoingPackets, or
     *          NULL if it does not exist.
     */
    Packet<T>* NextPacket();

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
    void PlaceInternalAtom(const T& atom, const SPoint& pt) {
      PlaceAtom(atom,pt+SPoint(R,R));
    }

    void FlushAndWaitOnAllBuffers(u32 dirWaitWord);

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
    u32 GetAtomCount(ElementType atomType);

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
     * Registers an Element into this Tile's ElementTable.
     *
     * @param the Element to register into this Tile's ElementTable.
     */
    void RegisterElement(const Element<T,R> & anElement) {
      elementTable.RegisterElement(anElement);
    }
  };
} /* namespace MFM */

#include "Tile.tcc"

#endif /*TILE_H*/

