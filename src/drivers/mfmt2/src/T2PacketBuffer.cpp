#include "T2PacketBuffer.h"
#include "T2Tile.h" /*for getDir6Name?*/


namespace MFM {
  //  namespace T2Packet {
    bool asTagSync(const T2PacketBuffer & pb, s32 * ptag) {
      u8 sn;
      if (asITC(pb,&sn) && (sn == ITCSN_SHUT || sn == ITCSN_OPEN)) {
        if (ptag) {
          u32 len = pb.GetLength(), tagsync;
          if (len != 6)
            FAIL(INCOMPLETE_CODE);
          CharBufferByteSource cbbs = pb.AsByteSource();
          cbbs.Scanf("%c%c%l",0,0,&tagsync);
          *ptag = tagsync;
        }
        return true;
      }
      return false;
    }


    void reportPacketAnalysis(const T2PacketBuffer & pb, ByteSink & bs) {
      u8 src, sn, cs, cn;
      s32 tag;
      if (!asStandardRouted(pb,&src)) {
        bs.Printf("!SR ");
        return;
      } 
      u8 dir6 = mapDir8ToDir6(src);
      if (asITC(pb, &sn)) {
        bs.Printf("i%s:%s",
                  getDir6Name(dir6),
                  getITCStateName((ITCStateNumber) sn));
      } else if (asCircuitSignal(pb,&cs,&cn)) {
        bs.Printf("%s:%s#%d",
                  getDir6Name(dir6),
                  getXITCName((XITCCode) cs),
                  cn);
        s8 dx, dy;
        bool yoink;
        u8 radius;
        if (asCSRing(pb, 0, &dx, &dy, &yoink, &radius)) {
          bs.Printf("(%d,%d)+%d%c",
                    (s32) dx, (s32) dy, (u32) radius,
                    yoink ? 'o' : 'e');
        }
      } else {
        bs.Printf("UNR");
      }
      if (asTagSync(pb, &tag)) {
        bs.Printf("?%08x ",tag);
      }
    }

    
    
  //  }
}
