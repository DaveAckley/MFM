#ifndef PACKET_H /* -*- C++ -*- */
#define PACKET_H

#include "eucliddir.h"
#include "itype.h"

namespace MFM {

  /**
   * An enumeration of all of the kinds of different Packets used
   * during Tile communication.
   */
  typedef enum
    {
      /** This type describes a Tile's intent to write an atom to a
	  specific location.*/
      PACKET_WRITE
    }PacketType;

  /**
   * An inter-Tile communication object which holds any and all
   * information that Tiles need to pass to one another.
   */
  template <class T>
  class Packet
  {
  private:
    /**
     * The PacketType of this particular packet. Upon receipt, a
     * Packet's data is interpreted solely from its PacketType.
     */
    PacketType m_type;

    /**
     * Used to describe a location during Tile communication.
     */
    SPoint m_edgeLoc;

    /**
     * Used to describe an edge during Tile communication.
     */
    EuclidDir m_toNeighbor;

    /**
     * Used to describe an Atom during Tile communication.
     */
    T m_atom;

  public:

    /**
     * Constructs a new Packet. Its PacketType must be known at time
     * of construction.
     */
    Packet(PacketType type);

    /**
     * Constructs an unitnitialized Packet.
     */
    Packet() { }

    /**
     * Gets the PacketType of this Packet.
     *
     * @returns The PacketType of this Packet.
     */
    PacketType GetType()
    { return m_type; }

    /**
     * Sets the PacketType of this Packet.
     *
     * @param type The new PacketType of this Packet.
     */
    void SetType(PacketType type)
    { m_type = type; }

    /**
     * Sets this Packet's held Atom.
     *
     * @param atom The Atom to copy into this Packet. A reference is
     *             not kept to this Atom.
     */
    void SetAtom(T& atom);

    /**
     * Sets this Packet's held EuclidDir neighbor field.
     *
     * @param dir The EuclidDir to place inside this Packet.
     */
    void SetReceivingNeighbor(EuclidDir dir)
    { m_toNeighbor = dir; }

    /**
     * Gets this Packet's held EuclidDir neighbor field.
     *
     * @returns This Packet's held Eucliddir neighbor field.
     */
    EuclidDir GetReceivingNeighbor()
    { return m_toNeighbor; }

    /**
     * Gets a reference to this Packet's held Atom.
     *
     * @returns A reference to this Packet's held Atom.
     */
    T& GetAtom();

    /**
     * Sets this Packet's held location.
     *
     * @param fromPt The SPoint to copy into this Packet.
     */
    void SetLocation(const SPoint& fromPt);

    /**
     * Gets this Packet's held location.
     *
     * @returns This Packet's held location.
     */
    const SPoint& GetLocation() const;
  };
} /* namespace MFM */

#include "packet.tcc"

#endif /*PACKET_H*/

