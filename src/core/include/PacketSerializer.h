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
#include "AtomSerializer.h"

namespace MFM
{
  template <class CC>
  class PacketSerializer : public ByteSerializable
  {
    typedef typename CC::PARAM_CONFIG P;
    typedef typename CC::ATOM_TYPE T;
    enum { BPA = P::BITS_PER_ATOM };

   private:
    Packet<T>& m_packet;

   public:
    PacketSerializer(Packet<T>& packet) :
      m_packet(packet)
    { }

    virtual Result PrintTo(ByteSink& byteSink, s32 argument = 0)
    {
      SPoint loc = m_packet.GetLocation();
      AtomSerializer<CC> atom(m_packet.GetAtom());
      byteSink.Printf("Pkt%d{%d/%s, gen:%d, nghb:%d, (%d, %d), %@}",
                      sizeof(Packet<T>),
                      (u32) m_packet.GetType(),
                      m_packet.GetTypeString(),
                      m_packet.GetGeneration(),
                      m_packet.GetReceivingNeighbor(),
                      loc.GetX(), loc.GetY(),
                      &atom);

      return SUCCESS;
    }

    virtual Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      return UNSUPPORTED;
    }
  };
}

#endif /* PACKETDECODER_H */
