#ifndef TILE_H      /* -*- C++ -*- */
#define TILE_H

#include "eucliddir.h"
#include "random.h"  /* for Random */
#include "packet.h"
#include "packetbuffer.h"
#include "point.h"
#include "eventwindow.h"
#include "elementtable.h"


namespace MFM {

  /** The full length, in sites, of a Tile, including neighbor
      caches.*/
#define TILE_WIDTH 40

  /** The number of sites a Tile contains. */
#define TILE_SIZE (TILE_WIDTH * TILE_WIDTH)

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

  /** A bitfield representing this Tile's connected neighbors. Used to
   *  find 'dead' caches, i.e. caches which aren't connected to a
   *  neighbor.
   */
  u8 m_neighborConnections;

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

  /** The PacketBuffer which will hold all outgoing Packets meant for
      other tiles.*/
  PacketBuffer<T> m_outgoingPackets;

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
   */
  void SendRelevantAtoms();

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

public:

  /**
   * Constructs a new Tile.
   */
  Tile();

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
   * Sets the neighbors bitfield to a new bitfield. This in essence
   * should only be called when constructing the Tile or when adding a
   * neighbor to the Tile to tell it which directions it is able to
   * send packets in.
   */
  void SetNeighbors(u8 neighbors);

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

  /*
   * Finds the cache pointed at by pt. If there
   * is no cache there, this will return -1.
   */

  /**
   * Finds the cache in this tile which contains a specified Point.
   *
   * @param pt The Point which should specify the location of a cache.
   *
   * @returns The direction of the cache specified by pt, or
   *          (EuclidDir)-1 if there is no such cache.
   */
  EuclidDir CacheAt(SPoint& pt);


  /**
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
  T* GetAtom(const SPoint& pt);

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
  T* GetAtom(int x, int y);

  /**
   * Gets an Atom from this Tile by its raster index.
   *
   * @param i The raster index (horizontal first) of the Atom to
   *          retrieve.
   *
   * @returns A pointer to the Atom at the specified index.
   */
  T* GetAtom(int i);

  /**
   * Processes a Packet, performing all necessary operations defined by
   * Packet symantics.
   *
   * @param packet The Packet which this Tile will process.
   */
  void ReceivePacket(Packet<T>& packet);

  /*
   * Gets the next packet that needs to be sent. This
   * returns NULL if there are no more packets to take.
   */

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

  /**
   * Executes a single new EventWindow at a randomly chosen location.
   */
  void Execute();

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
   * Adds an offset to the count of a particular type of Atom.
   *
   * @param atomType The ElementType of the Atoms of which the count
   *                 will change.
   *
   * @param delta The offset to add to the count of all Atoms of type
   *              atomType.
   */
  void IncrAtomCount(ElementType atomType, s32 delta);

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

#include "tile.tcc"

#endif /*TILE_H*/

