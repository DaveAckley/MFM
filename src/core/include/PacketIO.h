/*                                              -*- mode:C++ -*-
  PacketIO.h Support for writing and reading intertile packets
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
  \file PacketIO.h Support for writing and reading intertile packets
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PACKETIO_H
#define PACKETIO_H

#include "Packet.h"
#include "OverflowableCharBufferByteSink.h"
#include "EventConfig.h"
#include "AtomSerializer.h"
#include "itype.h"

namespace MFM
{
  template <class EC> class CacheProcessor; // FORWARD

  class PacketIO {
    PacketBuffer m_buffer;
  public:
    template <class EC>
    bool SendUpdateBegin(CacheProcessor<EC> & cxn, const SPoint & localCenter) ;

    template <class EC>
    bool SendUpdateEnd(CacheProcessor<EC> & cxn) ;

    template <class EC>
    bool SendAtom(PacketTypeCode ptype, CacheProcessor<EC> & cxn,
                  u16 siteNumber, const typename EC::ATOM_CONFIG::ATOM_TYPE & atom) ;

    template <class EC>
    bool SendReply(PacketTypeCode ptype, CacheProcessor<EC> & cxn) ;

    /**
       Parse (and dispatch to ReceiveXXX methods herein) to deal with
       the packet in buf.  \returns true if all went well, \returns
       false if some problem.
     */
    template <class EC>
    bool HandlePacket(CacheProcessor<EC> & cxn, PacketBuffer & buf) ;

    template <class EC>
    bool ReceiveUpdateBegin(CacheProcessor<EC> & cxn, ByteSource & buf) ;

    template <class EC>
    bool ReceiveAtom(CacheProcessor<EC> & cxn, ByteSource & buf) ;

    template <class EC>
    bool ReceiveUpdateEnd(CacheProcessor<EC> & cxn, ByteSource & buf) ;

    template <class EC>
    bool ReceiveReply(CacheProcessor<EC> & cxn, ByteSource & buf) ;

  };

} /* namespace MFM */

#include "PacketIO.tcc"

#endif /*PACKETIO_H*/
