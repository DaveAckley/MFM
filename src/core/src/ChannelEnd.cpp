#include "ChannelEnd.h"
#include "PacketIO.h"

namespace MFM
{
  void ChannelEnd::ReportChannelEndStatus(Logger::Level level)
  {
    LOG.Log(level,"    ==ChannelEnd %p [chn %p] ==",
            (void*) this,
            (void*) m_channel);
    LOG.Log(level,"     PacketLength: %d", m_packetLength);
    LOG.Log(level,"     Pending length: %d", m_packetBuffer.GetLength());
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
