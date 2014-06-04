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

#include "Packet.h"
#include "itype.h"

namespace MFM {

#define PACKETBUFFER_SIZE 128

template <class T>
class PacketBuffer
{
private:
  Packet<T> m_buffer[PACKETBUFFER_SIZE];

  u32 m_heldPackets;

public:

  PacketBuffer();

  void PushPacket(Packet<T>& packet);

  Packet<T>* PopPacket();

  u32 PacketsHeld()
  { return m_heldPackets; }

};
} /* namespace MFM */

#include "PacketBuffer.tcc"

#endif /*PACKETBUFFER_H*/
