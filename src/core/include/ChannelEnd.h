/*                                              -*- mode:C++ -*-
  ChannelEnd.h A termination point for an intertile channel
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
  \file ChannelEnd.h A termination point for an intertile channel
  \author David H. Ackley
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CHANNELEND_H
#define CHANNELEND_H

#include "itype.h"
#include "Fail.h"
#include "Logger.h"
#include "AbstractChannel.h"
#include "LonglivedLock.h"
#include "OverflowableCharBufferByteSink.h"
#include "Packet.h"

namespace MFM
{
  class PacketIO; // FORWARD

  /**
    One end of an AbstractChannel, plus associated information.  A
    ChannelEnd facilitates reading and writing the channel, and
    maintains cache statistics enabling adaptive redundancy.
   */
  class ChannelEnd {

    /**
       Incoming packet buffer.  Bytes read from m_channel are stored
       here until a complete packet has been received; reading then
       stops until a ChannelEnd user calls ReceivePacket.
     */
    PacketBuffer m_packetBuffer;

    /**
       Total packet length of the packet (being) received into
       m_packetBuffer, or -1 to indicate no packet has currently been
       started.
     */
    s32 m_packetLength;

    /**
       The channel transporting bytes to and from the far side
     */
    AbstractChannel * m_channel;

    /**
       Symmetry breaker determining which role this end is playing in
       the AbstractChannel.  If true, this end is side 'A' of the
       AbstractChannel; otherwise it is side 'B'.
     */
    bool m_onSideA;

    /**
       [NOT USED, POLLING FOR INBOUND?] An identifying tag for the
       current owner of this channel end.  Used to route inbound reply
       packets.  -1 means no owner.
     */
    s32 m_owner;

  public:
    void ReportChannelEndStatus(Logger::Level level);

    void AssertConnected() const
    {
      if (!m_channel)
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    bool IsConnected() const
    {
      return m_channel != 0;
    }

    u32 CanWrite()
    {
      AssertConnected();
      return m_channel->CanWrite(m_onSideA);
    }

    u32 Write(const u8 * data, u32 length)
    {
      AssertConnected();
      return m_channel->Write(m_onSideA, data, length);
    }

    u32 CanRead()
    {
      AssertConnected();
      return m_channel->CanRead(m_onSideA);
    }

    s32 ReadByte()
    {
      u8 byte;
      if (Read(&byte, 1) < 1)
      {
        return -1;
      }
      return byte;
    }

    u32 Read(u8 * data, u32 length)
    {
      AssertConnected();
      return m_channel->Read(m_onSideA, data, length);
    }

    s32 GetOwner() const
    {
      AssertConnected();
      return m_owner;
    }

    void SetOwner(s32 tag)
    {
      AssertConnected();
      if ((m_owner < 0) == (tag < 0))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      m_owner = tag;
    }

    void ClaimChannelEnd(AbstractChannel& channel, bool onSideA)
    {
      if (m_channel != 0)
      {
        FAIL(ILLEGAL_STATE);
      }
      m_channel = &channel;
      m_onSideA = onSideA;
    }

    /**
       Return NULL if no complete packet is available.  Otherwise
       return a pointer to a buffered, complete, unparsed packet.
       When ReceivePacket() returns non-NULL, caller must finish with
       the contents of the returned PacketBuffer before the next call
       to ReceivePacket.  The returned pointer is statically allocated
       and owned by ChannelEnd; caller must not free or otherwise mess
       with it except to read its contents.
     */
    PacketBuffer * ReceivePacket() ;

    ChannelEnd()
      : m_packetLength(-1)
      , m_channel(0)
      , m_onSideA(false)
      , m_owner(-1)
    { }
  };
}

#endif /* CHANNELEND_H */
