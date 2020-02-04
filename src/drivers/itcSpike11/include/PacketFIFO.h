/* -*- C++ -*- */
#ifndef PACKETFIFO_H
#define PACKETFIFO_H

#include "itype.h"
#include "Packet.h"

#define MAX_PACKET_SIZE 256
#define PACKET_FIFO_BITS 12
#define PACKET_FIFO_LEN (1<<PACKET_FIFO_BITS)
#define PACKET_FIFO_MASK (PACKET_FIFO_LEN-1)

namespace MFM {
  struct PacketFIFO {
    u32 mWritePtr;
    u32 mReadPtr;
    u32 mPacketsRejected;
    u32 mPacketsAdded;
    u32 mPacketsRemoved;
    u8 mBuffer[PACKET_FIFO_LEN];
    
    PacketFIFO()
      : mWritePtr(0)
      , mReadPtr(0)
      , mPacketsRejected(0)
      , mPacketsAdded(0)
      , mPacketsRemoved(0)
    { }

    inline u32 bytesUsed() const {
      s32 diff = mWritePtr - mReadPtr;
      if (diff < 0) diff += PACKET_FIFO_LEN;
      return (u32) diff;
    }

    inline u32 bytesAvailable() const {
      return PACKET_FIFO_LEN - bytesUsed() - 1;
    }
    
    inline bool isEmpty() const { return bytesUsed() == 0; }
    inline bool isFull() const { return bytesAvailable() == 0; }

    inline bool storeByte(u8 byte) {
      if (isFull()) return false;
      mBuffer[mWritePtr] = byte;
      mWritePtr = (mWritePtr+1) & PACKET_FIFO_MASK;
      return true;
    }

    inline s32 getFrontPacketStartIndex() const {
      if (isEmpty()) return -1;
      return (mReadPtr + 1) & PACKET_FIFO_MASK;
    }

    inline u8 getFrontPacketByte(u8 packetIdx) {
      s32 base = getFrontPacketStartIndex();
      if (base < 0) return 0; // XXX How to fail?
      u32 idx = (((u32) base) + packetIdx) & PACKET_FIFO_MASK;
      return mBuffer[idx];
    }

    bool addPacket(const u8 * packet, u32 len) ;

    bool addPacketBuffer(PacketBuffer & pb) {
      return addPacket((const u8 *) pb.GetBuffer(), pb.GetLength());
    }
    bool removePacketBuffer(PacketBuffer &pb) ;

    bool peekFrontPacketType(u8 & type) ;

    u32 frontPacketLen() const {
      if (isEmpty()) return 0;
      return mBuffer[mReadPtr];
    }

    bool dropFrontPacket() {
      u32 len = frontPacketLen();
      if (len == 0) return false;
      mReadPtr = (mReadPtr + len + 1) & PACKET_FIFO_MASK;
      return true;
    }

    bool removePacket(u8 (&buffer)[MAX_PACKET_SIZE], u8 & len) ;
    
  };
}

#endif /* PACKETFIFO_H */
