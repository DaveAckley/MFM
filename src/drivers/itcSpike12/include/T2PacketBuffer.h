/* -*- C++ -*- */
#ifndef T2PACKETBUFFER_H
#define T2PACKETBUFFER_H

#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  typedef OString256 T2PacketBuffer;

  typedef enum xitccode {
    XITC_KITC_CMD=0,
    XITC_ITC_CMD=1,
    XITC_CS_MIN_VAL=2,
    XITC_CS_RING=2,
    XITC_CS_ANSWER=3,
    XITC_CS_BUSY=4,
    XITC_CS_DROP=5,
    XITC_CS_TALK=6,
    XITC_CS_HANGUP=7,
    XITC_CS_MAX_VAL=7,
  } XITCCode;

  namespace T2Packet {
    inline const char * getXITCName(XITCCode xitc) {
      switch (xitc) {
      default: return "illegal xitc";
      case XITC_KITC_CMD: return "KITC";
      case XITC_ITC_CMD: return "ITC";
      case XITC_CS_RING: return "RING";
      case XITC_CS_ANSWER: return "ANSWER";
      case XITC_CS_BUSY: return "BUSY";
      case XITC_CS_DROP: return "DROP";
      case XITC_CS_TALK: return "TALK";
      case XITC_CS_HANGUP: return "HANGUP";
      }
    }

    inline bool isStandard(const T2PacketBuffer & pb) {
      return pb.GetLength() > 0 && (pb.GetBuffer()[0]&0x80) != 0;
    }
    inline bool asNonstandard(const T2PacketBuffer & pb, u8 * ascii7) {
      if (isStandard(pb)) return false;
      if (ascii7 != 0) *ascii7 = pb.GetBuffer()[0];
      return true;
    }
    inline bool asStandardLocal(const T2PacketBuffer & pb, u8 * type) {
      if (pb.GetLength() > 0 && (pb.GetBuffer()[0]&0xc0) == 0xc0) {
        if (type) *type = (pb.GetBuffer()[0]&0x1f);
        return true;
      }
      return false;
    }
    inline bool isStandardRouted(const T2PacketBuffer & pb) {
      return pb.GetLength() > 0 && (pb.GetBuffer()[0]&0xc0) == 0x80;
    }
    inline bool asStandardRouted(const T2PacketBuffer & pb, u8 * dir8) {
      if (isStandardRouted(pb)) {
        if (dir8) *dir8 = pb.GetBuffer()[0]&0x07;
        return true;
      }
      return false;
    }
    inline bool isStandardBulk(const T2PacketBuffer & pb) {
      return isStandardRouted(pb) && (pb.GetBuffer()[0]&0xe0)==0x80;
    }
    inline bool isStandardUrgent(const T2PacketBuffer & pb) {
      return isStandardRouted(pb) && (pb.GetBuffer()[0]&0xe0)==0xa0;
    }
    inline bool isStandardFlash(const T2PacketBuffer & pb) {
      return pb.GetLength() > 1 && isStandardUrgent(pb) && (pb.GetBuffer()[1]&0x80)==0x00;
    }
    inline bool isStandardMFM(const T2PacketBuffer & pb) {
      return pb.GetLength() > 1 && isStandardUrgent(pb) && (pb.GetBuffer()[1]&0x80)==0x80;
    }
    inline bool asXITC(const T2PacketBuffer & pb, u8 * xitc) {
      if (isStandardMFM(pb)) {
        if (xitc) *xitc = ((pb.GetBuffer()[1]>>4)&0x7);
        return true;
      }
      return false;
    }
    inline bool asITC(const T2PacketBuffer & pb, u8 * sn) {
      u8 xitc; 
      if (asXITC(pb,&xitc) && xitc == 1) {
        if (sn) *sn = (pb.GetBuffer()[1]&0xf);
        return true;
      }
      return false;
    }
    inline bool asCircuitSignal(const T2PacketBuffer & pb, u8 *cs, u8 *cn) {
      u8 xitc;
      if (asXITC(pb,&xitc) && xitc >= 2) {
        if (cs) *cs = xitc;
        if (cn) *cn = (pb.GetBuffer()[1]&0xf);
        return true;
      }
      return false;
    }
    template <XITCCode c>
    inline bool asCSType(const T2PacketBuffer &pb, u8 *pcn) {
      u8 cs, cn;
      if (!asCircuitSignal(pb,&cs,&cn)) return false;
      if (cs != c) return false;
      if (pcn) *pcn = cn;
      return true;
    }
    inline bool asCSRing(const T2PacketBuffer &pb, u8 *pcn, s8 *cx, s8 *cy, bool * yoink, u8 * radius) {
      u8 tcn;
      if (!asCSType<XITC_CS_RING>(pb, &tcn)) return false;
      if (pb.GetLength() < 5) return false;
      if (pcn) *pcn = tcn;
      if (cx) *cx = (s8) pb.GetBuffer()[2];
      if (cy) *cy = (s8) pb.GetBuffer()[3];
      if (yoink) *yoink = (((pb.GetBuffer()[4]>>7)&1)!=0);
      if (radius) *radius = (pb.GetBuffer()[4]&0x7);
      return true;
    }
    inline bool asCSAnswer(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_ANSWER>(pb, pcn); }
    inline bool asCSBusy(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_BUSY>(pb, pcn); }
    inline bool asCSDrop(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_DROP>(pb, pcn); }
    inline bool asCSTalk(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_TALK>(pb, pcn); }
    inline bool asCSHangup(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_HANGUP>(pb, pcn); }

    bool asTagSync(const T2PacketBuffer & pb, s32 * ptag) ;
    
    void reportPacketAnalysis(const T2PacketBuffer & pb, ByteSink & bs) ;
  }
}
#endif /* T2PACKETBUFFER_H */
