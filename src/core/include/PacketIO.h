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
#include "CoreConfig.h"
#include "AtomSerializer.h"
#include "itype.h"

namespace MFM
{
  template <class CC> class CacheProcessor; // FORWARD

  class PacketIO {
    PacketBuffer m_buffer;
  public:
    template <class CC>
    bool SendUpdateBegin(CacheProcessor<CC> & cxn, const SPoint & localCenter) ;

    template <class CC>
    bool SendUpdateEnd(CacheProcessor<CC> & cxn) ;

    template <class CC>
    bool SendAtom(PacketTypeCode ptype, CacheProcessor<CC> & cxn,
                  u16 siteNumber, const typename CC::ATOM_TYPE & atom) ;

    template <class CC>
    bool SendReply(PacketTypeCode ptype, CacheProcessor<CC> & cxn) ;

    /**
       Parse (and dispatch to ReceiveXXX methods herein) to deal with
       the packet in buf.  \returns true if all went well, \returns
       false if some problem.
     */
    template <class CC>
    bool HandlePacket(CacheProcessor<CC> & cxn, PacketBuffer & buf) ;

    template <class CC>
    bool ReceiveUpdateBegin(CacheProcessor<CC> & cxn, ByteSource & buf) ;

    template <class CC>
    bool ReceiveAtom(CacheProcessor<CC> & cxn, ByteSource & buf) ;

    template <class CC>
    bool ReceiveUpdateEnd(CacheProcessor<CC> & cxn, ByteSource & buf) ;

    template <class CC>
    bool ReceiveReply(CacheProcessor<CC> & cxn, ByteSource & buf) ;

  };

} /* namespace MFM */

#include "PacketIO.tcc"

#endif /*PACKETIO_H*/
