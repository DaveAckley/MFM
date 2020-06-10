#include "PacketFIFO.h"

namespace MFM {
  bool PacketFIFO::addPacket(const u8 * packet, u32 len) {
    if (!packet
        || len == 0
        || len >= MAX_PACKET_SIZE
        || len >= bytesAvailable()) {
      ++mPacketsRejected;
      return false;
    }
    storeByte((u8) len);
    for (u32 i = 0; i < len; ++i) storeByte(packet[i]);
    ++mPacketsAdded;
    return true;
  }

  bool PacketFIFO::peekFrontPacketType(u8 & type) {
    if (bytesUsed() < 2) return false;
    type = mBuffer[(mReadPtr+1) & PACKET_FIFO_MASK];
    return true;
  }

  bool PacketFIFO::removePacket(u8 (&buffer)[MAX_PACKET_SIZE], u8 & len) {
    u8 plen = frontPacketLen();
    if (plen == 0) return false;
    for (u32 i = 0; i < plen; ++i)
      buffer[i] = getFrontPacketByte(i);
    dropFrontPacket();
    len = plen;
    return true;
  }

  bool PacketFIFO::removePacketBuffer(PacketBuffer & pb) {
    u8 plen = frontPacketLen();
    if (plen == 0) return false;
    pb.Reset();
    for (u32 i = 0; i < plen; ++i)
      pb.WriteByte(getFrontPacketByte(i));
    dropFrontPacket();
    return true;
  }

}
