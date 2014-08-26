/*                                              -*- mode:C++ -*-
  AbstractDriver.h String translator for Packets
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
  \file PacketSerializer.h String translator for Packets
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PACKETDECODER_H
#define PACKETDECODER_H

#include "ByteSerializable.h"
#include "Packet.h"

namespace MFM
{
  template <class T>
  class PacketSerializer : public ByteSerializable
  {
   private:
    Packet<T>& m_packet;

   public:
    PacketSerializer(Packet<T>& packet) :
      m_packet(packet)
    { }

    virtual Result PrintTo(ByteSink& byteSink, s32 argument = 0)
    {
      SPoint loc = m_packet.GetLocation();
      byteSink.Printf("Packet{size=%d type=%s generation=%d Location=(%d, %d)}",
                      sizeof(Packet<T>),
                      m_packet.GetTypeString(),
                      m_packet.GetGeneration(),
                      loc.GetX(), loc.GetY());

      return SUCCESS;
    }
  };
}

#endif /* PACKETDECODER_H */
