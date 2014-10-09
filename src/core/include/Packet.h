/*                                              -*- mode:C++ -*-
  Packet.h Basic type for inter-tile communication
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
  \file Packet.h Basic type for inter-tile communication
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PACKET_H
#define PACKET_H

#include "Dirs.h"
#include "itype.h"

namespace MFM
{

  /**
   * An enumeration of all of the kinds of different Packets used
   * during Tile communication.
   */
  typedef enum
    {
      /**
       * This PacketType describes a Tile's intent to write an atom to
       * a specific location.
       */
      PACKET_WRITE = 0,

      /**
       * Used to keep inter-tile buffers from overflowing, this PacketType
       * describes that a Tile has completed processing an event and
       * is ready to unlock its caches.
       */
      PACKET_EVENT_COMPLETE,

      /**
       * Used to keep inter-tile buffers from overflowing, this
       * PacketType describes that a Tile has completed all event
       * processing from another Tile's events. In practice, a Packet
       * of this PacketType should be sent immediately upon receipt
       * of a PACKET_EVENT_COMPLETE Packet.
       */
      PACKET_EVENT_ACKNOWLEDGE

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
    u8 m_type;

    /**
     * Used to describe an edge during Tile communication.
     */
    u8 m_toNeighbor;

    /**
     * Used to discard obsolete packets after tile resets.
     */
    u8 m_generation;

    /**
     * Used to describe a location during Tile communication.
     */
    SSPoint m_edgeLoc;

    /**
     * Used to describe an Atom during Tile communication.
     */
    T m_atom;

  public:

    /**
     * Constructs a new Packet of a given PacketType.
     */
    Packet(PacketType type, u8 generation) :
      m_type(type),
      m_toNeighbor(Dirs::DIR_COUNT), // Init invalid
      m_generation(generation)
    { }

    const char* GetTypeString()
    {
      switch(m_type)
      {
      case PACKET_WRITE: return "Write";
      case PACKET_EVENT_COMPLETE: return "Event Complete";
      case PACKET_EVENT_ACKNOWLEDGE: return "Event Acknowledge";
      }
      return "INVALID";
    }

    /**
     * Gets the PacketType of this Packet.
     *
     * @returns The PacketType of this Packet.
     */
    PacketType GetType()
    {
      return (PacketType) m_type;
    }

    /**
     * Sets the PacketType of this Packet.
     *
     * @param type The new PacketType of this Packet.
     */
    void SetType(PacketType type)
    {
      m_type = type;
    }

    /**
     * Sets this Packet's held Atom.
     *
     * @param atom The Atom to copy into this Packet. A reference is
     *             not kept to this Atom.
     */
    void SetAtom(const T& atom)
    {
      m_atom = atom;
    }

    /**
     * Sets this Packet's held EuclidDir neighbor field.
     *
     * @param dir The EuclidDir to place inside this Packet.
     */
    void SetReceivingNeighbor(Dir dir)
    {
      m_toNeighbor = dir;
    }

    /**
     * Gets this Packet's held EuclidDir neighbor field.
     *
     * @returns This Packet's held Eucliddir neighbor field.
     */
    u32 GetReceivingNeighbor()
    {
      return m_toNeighbor;
    }

    /**
     * Gets a reference to this Packet's held Atom.
     *
     * @returns A reference to this Packet's held Atom.
     */
    T& GetAtom()
    {
      return m_atom;
    }

    /**
     * Sets this Packet's held location.
     *
     * @param fromPt The SPoint to copy into this Packet.
     */
    void SetLocation(const SPoint& fromPt)
    {
      m_edgeLoc = SSPoint(fromPt.GetX(), fromPt.GetY());
    }

    /**
     * Gets this Packet's held location.
     *
     * @returns This Packet's held location.
     */
    const SPoint GetLocation() const
    {
      return SPoint(m_edgeLoc.GetX(), m_edgeLoc.GetY());
    }

    /**
     * Gets this Packet's generation.
     *
     * @returns This Packet's generation.
     */
    const u8 GetGeneration() const
    {
      return m_generation;
    }

    /**
     * Gets this Packet's generation.
     *
     * @returns This Packet's generation.
     */
    void SetGeneration(u8 generation)
    {
      m_generation = generation;
    }

    /**
     * Determine if this packet is obsolete compared to \c
     * ourGeneration.
     *
     * @returns true iff this packet's generation should be considered
     * 'less than' \c ourGeneration considering u8 wraparound.
     */
    bool IsObsolete(u8 ourGeneration) const
    {
      return ((u8) (m_generation - ourGeneration)) >= U8_MAX / 2;
    }
  };
} /* namespace MFM */

#endif /*PACKET_H*/
