/* -*- C++ -*- */
#ifndef T2PACKETBUFFER_H
#define T2PACKETBUFFER_H

#include "OverflowableCharBufferByteSink.h"

/*LKM files*/
#include "itcpktevent.h"

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

  typedef enum ringbyte4code {
    RING_BYTE4_BITMASK_YOINK = 0x80,                              
    RING_BYTE4_YOINK_POS     = __builtin_ctz(RING_BYTE4_BITMASK_YOINK),
    RING_BYTE4_BITMASK_RDC   = 0x60,                              
    RING_BYTE4_RDC_POS       = __builtin_ctz(RING_BYTE4_BITMASK_RDC)
  } RINGByte4Code;

  inline u8 ringByte4(u8 yoinkBit, u8 radius) {
    return
      (u8) (((yoinkBit<<RING_BYTE4_YOINK_POS) & RING_BYTE4_BITMASK_YOINK) |
            ((radius-1)<<RING_BYTE4_RDC_POS) & RING_BYTE4_BITMASK_RDC);
  }

  //  namespace T2Packet {
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
      return
        pb.GetLength() > 0 &&
        (pb.GetBuffer()[0] & PKT_HDR_BITMASK_STANDARD) != 0;
    }
    inline bool asNonstandard(const T2PacketBuffer & pb, u8 * ascii7) {
      if (isStandard(pb)) return false;
      if (ascii7 != 0) *ascii7 = pb.GetBuffer()[0];
      return true;
    }
    inline bool asStandardLocal(const T2PacketBuffer & pb, u8 * type) {
      if (pb.GetLength() > 0 &&
          ((pb.GetBuffer()[0] & PKT_HDR_BITMASK_STANDARD_LOCAL) ==
           PKT_HDR_BITMASK_STANDARD_LOCAL)) {
        if (type) *type = (pb.GetBuffer()[0] & PKT_HDR_BITMASK_LOCAL_TYPE);
        return true;
      }
      return false;
    }
    inline bool isStandardRouted(const T2PacketBuffer & pb) {
      return
        pb.GetLength() > 0 &&
        ((pb.GetBuffer()[0] & PKT_HDR_BITMASK_STANDARD_LOCAL) ==
         PKT_HDR_BITMASK_STANDARD);
    }
    inline bool asStandardRouted(const T2PacketBuffer & pb, u8 * dir8) {
      if (isStandardRouted(pb)) {
        if (dir8) *dir8 = pb.GetBuffer()[0] & PKT_HDR_BITMASK_DIR;
        return true;
      }
      return false;
    }
    inline bool isStandardService(const T2PacketBuffer & pb) {
      return
        isStandardRouted(pb) &&
        ((pb.GetBuffer()[0] & PKT_HDR_BITMASK_MFM) == 0);
    }
    inline bool isStandardMFM(const T2PacketBuffer & pb) {
      return
        pb.GetLength() > 1 &&
        isStandardRouted(pb) &&
        (pb.GetBuffer()[0] & PKT_HDR_BITMASK_MFM);
    }
    inline bool isStandardFlash(const T2PacketBuffer & pb) {
      return
        pb.GetLength() > 1 && 
        isStandardService(pb) &&
        ((pb.GetBuffer()[1] & PKT_HDR_BYTE1_BITMASK_BULK) == 0);
    }
    inline bool isStandardBulk(const T2PacketBuffer & pb) {
      return
        pb.GetLength() > 1 &&
        isStandardService(pb) &&
        (pb.GetBuffer()[1] & PKT_HDR_BYTE1_BITMASK_BULK);
    }
    inline bool asXITC(const T2PacketBuffer & pb, u8 * xitc) {
      if (isStandardMFM(pb)) {
        if (xitc)
          *xitc =
            ((pb.GetBuffer()[1] >> PKT_HDR_BYTE1_XITC_POS) &
             (PKT_HDR_BYTE1_BITMASK_XITC >> PKT_HDR_BYTE1_XITC_POS));
        return true;
      }
      return false;
    }
    inline bool asITC(const T2PacketBuffer & pb, u8 * sn) {
      u8 xitc; 
      if (asXITC(pb,&xitc) && xitc == XITC_ITC_CMD) {
        if (sn) *sn = (pb.GetBuffer()[1] & PKT_HDR_BYTE1_BITMASK_XITC_SN);
        return true;
      }
      return false;
    }
    inline bool asCircuitSignal(const T2PacketBuffer & pb, u8 *cs, u8 *cn) {
      u8 xitc;
      if (asXITC(pb,&xitc) &&
          xitc >= XITC_CS_MIN_VAL &&
          xitc <= XITC_CS_MAX_VAL) {
        if (cs) *cs = xitc;
        if (cn) *cn = (pb.GetBuffer()[1] & PKT_HDR_BYTE1_BITMASK_XITC_SN);
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
      if (yoink)
        *yoink = (((pb.GetBuffer()[4]>>RING_BYTE4_YOINK_POS) & 0x1) != 0);
      if (radius)
        *radius = ((pb.GetBuffer()[4]>>RING_BYTE4_RDC_POS) &
                   (RING_BYTE4_BITMASK_RDC >> RING_BYTE4_RDC_POS)) + 1;
      return true;
    }
    inline bool asCSAnswer(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_ANSWER>(pb, pcn); }
    inline bool asCSBusy(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_BUSY>(pb, pcn); }
    inline bool asCSDrop(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_DROP>(pb, pcn); }
    inline bool asCSTalk(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_TALK>(pb, pcn); }
    inline bool asCSHangup(const T2PacketBuffer &pb, u8 *pcn) { return asCSType<XITC_CS_HANGUP>(pb, pcn); }

    bool asTagSync(const T2PacketBuffer & pb, s32 * ptag) ;
    
    void reportPacketAnalysis(const T2PacketBuffer & pb, ByteSink & bs) ;
  //  }
}
#endif /* T2PACKETBUFFER_H */
