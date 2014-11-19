#include "ChannelEnd.h"
#include "PacketIO.h"

namespace MFM
{
  const char * GetChannelStateName(u32 state)
  {
    switch (state)
    {
    case AbstractChannel::CHANNEL_UNOWNED: return "UNOWNED";
    case AbstractChannel::CHANNEL_A_OWNER: return "A OWNER";
    case AbstractChannel::CHANNEL_B_OWNER: return "B OWNER";
    default: return "illegal state";
    }
  }
  void ChannelEnd::ReportChannelEndStatus(Logger::Level level)
  {
    LOG.Log(level,"    ==ChannelEnd %p [chn %p] SIDE %s==",
            (void*) this,
            (void*) m_channel,
            m_onSideA ? "A" : "B");
    LOG.Log(level,"     PacketLength: %d", m_packetLength);
    LOG.Log(level,"     Pending length: %d", m_packetBuffer.GetLength());
    if (m_channel)
    {
      AbstractChannel::State acs = m_channel->GetChannelState();
      const char * who;
      if (acs == AbstractChannel::CHANNEL_UNOWNED)
      {
        who = "";
      }
      else
      {
        if ((acs == AbstractChannel::CHANNEL_A_OWNER) == m_onSideA)
        {
          who = "(US)";
        }
        else
        {
          who = "(THEM)";
        }
      }

      LOG.Log(level,"     State %s %s",
              GetChannelStateName(acs),
              who);
      LOG.Log(level,"     Side A: CW %d, CR %d",
              m_channel->CanWrite(true),
              m_channel->CanRead(true));
      LOG.Log(level,"     Side B: CW %d, CR %d",
              m_channel->CanWrite(false),
              m_channel->CanRead(false));
    }
  }

  PacketBuffer * ChannelEnd::ReceivePacket()
  {
    // Step 1: If a packet has not been started, try to start it
    if (m_packetLength < 0)
    {
      s32 byte = ReadByte();
      if (byte < 0)
      {
        return 0;              // Nothing there..
      }
      m_packetLength = byte;   // OK, packet started!
      m_packetBuffer.Reset();
    }

    // Step 2: If a packet is not yet finished, try to read enough to finish it
    while (m_packetBuffer.GetLength() < (u32) m_packetLength)
    {
      s32 byte = ReadByte();
      if (byte < 0)
      {
        return 0;              // Split packet.  Well damn.  Later.
      }
      m_packetBuffer.Print((u32) byte, Format::BYTE);
    }

    // Step 3: Privately mark packet done; let caller see what we got
    m_packetLength = -1;
    return & m_packetBuffer;
  }
}
