/*                                              -*- mode:C++ -*-
  Packet.h Basic definitions for inter-tile communication
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
  \file Packet.h Basic definitions for inter-tile communication
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PACKET_H
#define PACKET_H

#include "Dirs.h"
#include "itype.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM
{

  /**
     The type of variable that can contain a raw, unparsed Packet
   */
  typedef OString128 PacketBuffer;

  /**
     The type of a variable that can contain a PacketType value
   */
  typedef u8 PacketTypeCode;

  /**
     Constants for all the kinds of different Packets used during Tile
     communication.
   */
  namespace PacketType
  {
    /**
     * The PacketType when an updater is initiating a new cache
     * update, supplying the event window center coordinate mapped
     * into the updatee's full untransformed coordinate space.
     * Format: UPDATE_BEGIN + s16:CX + s16:CY
     */
    static const u8 UPDATE_BEGIN = 'b';

    /**
     * The PacketType when an updater is supplying a new value for a
     * site. Format: UPDATE + u8:SITENO + T:ATOM
     */
    static const u8 UPDATE = 'u';

    /**
     * The PacketType when an updater is supplying an old value for
     * a site as a redundant consistency spot-check.  Format:
     * CHECK + u8:SITENO + T:ATOM
     */
    static const u8 CHECK = 'c';

    /**
     * The PacketType when an updater has sent all packets it intends
     * to for this cache update. Format: UPDATE_END
     */
    static const u8 UPDATE_END = 'e';

    /**
     * The PacketType when an updatee has received an UPDATE_END.
     * Format: UPDATE_ACK + u8:CONSISTENT_ATOM_COUNT
     */
    static const u8 UPDATE_ACK = 'a';

  } /* namespace PacketType */

} /* namespace MFM */

#endif /*PACKET_H*/
