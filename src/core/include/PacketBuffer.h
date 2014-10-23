/*                                              -*- mode:C++ -*-
  PacketBuffer.h List for holding inter-tile IO Packets
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
  \file PacketBuffer.h List for holding inter-tile IO Packets
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PACKETBUFFER_H
#define PACKETBUFFER_H

#include "Fail.h"
#include "Packet.h"
#include "itype.h"

namespace MFM
{

  /**
   * The capacity of a PacketBuffer , in bytes.
   */
#define PACKETBUFFER_SIZE 128

  /**
   * A LIFO buffer designed to hold a Packet array . This is not
   * a thread safe structure.
   */
  template <class T>
  class PacketBuffer
  {
  private :
    /**
     * The internal Packet array held by this PacketBuffer .
     */
    Packet<T> m_buffer[PACKETBUFFER_SIZE];

    /**
     * The number of Packets currently held by this PacketBuffer .
     */
    u32 m_heldPackets;

    /**
     * The maximum number of Packets which may fit inside this
     * PacketBuffer before it is full.
     */
    const u32 m_packetCapacity;

  public:

    /**
     * Constructs a new PacketBuffer with an empty Packet array.
     */
    PacketBuffer();

    /**
     * Pushes a Packet to the top of the internal Packet array .  FAiLs
     * with OUT_OF_ROOM if there is not room for another Packet on the
     * stack.
     *
     * @param packet The Packet to put on top of the internal buffer.
     */
    void PushPacket(Packet<T>& packet);

    /**
     * Gets a pointer to the next Packet on the stack. FAILs with
     * ARRAY_INDEX_OUT_OF_BOUNDS if there is not a Packet left.
     *
     * @returns A pointer to the next Packet on the stack.
     */
    Packet<T>* PopPacket();

    /**
     * Gets the number of Packets currently held by this PacketBuffer .
     *
     * @returns The number of Packets currently held by this PacketBuffer .
     */
    u32 PacketsHeld()
    {
      return m_heldPackets;
    }
  };
} /* namespace MFM */

#include "PacketBuffer.tcc"

#endif /*PACKETBUFFER_H*/
