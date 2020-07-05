/* -*- C++ -*- */
#ifndef ITCSTATUS_H
#define ITCSTATUS_H

#include "itype.h"
#include "ByteSource.h"
#include "Point.h"
#include "Drawing.h"

#include "dirdatamacro.h"

#include "ITCIcons.h"
#include "T2VizConstants.h"
#include "T2Tile.h"

namespace MFM {

  struct ITCStatus {

    static const u32 (iconFaceIdx[DIR6_COUNT]);
    static const UPoint (screenPos[DIR6_COUNT]);
    static const SPoint (side1Pos[DIR6_COUNT]);
    static const SPoint (side2Pos[DIR6_COUNT]);

    const char * getAbbr() const { return getDir6Name(mDir6Idx); }

    // Note this IDXs are excluding the dir that begins each line
    enum {
      PACKET_SYNCS_IDX = 0,
      SYNC_FAILS_IDX,
      TIMEOUTS_IDX,
      MFM_BYTES_SENT_IDX,
      MFM_BYTES_RCVD_IDX,
      MFM_PACKETS_SENT_IDX,
      MFM_PACKETS_RCVD_IDX,
      SVC_BYTES_SENT_IDX,
      SVC_BYTES_RCVD_IDX,
      SVC_PACKETS_SENT_IDX,
      SVC_PACKETS_RCVD_IDX,
      ITC_STAT_COUNT
    };
    u32 mDir6Idx; // 0..5
    bool mIsOpen;
    bool mIsAlive;
    u32 mStats[ITC_STAT_COUNT];
    u32 mDeltas[ITC_STAT_COUNT];

    const u32 getIconFaceIdx() const { return iconFaceIdx[mDir6Idx]; }
    const UPoint getScreenPos() const { return screenPos[mDir6Idx]; }
    const SPoint getSide1Offset() const { return side1Pos[mDir6Idx]; }
    const SPoint getSide2Offset() const { return side2Pos[mDir6Idx]; }
    const bool isVertical() const { return mDir6Idx == 0 || mDir6Idx == 3; }

    u32 mapBytesToSizeIdx(u32 bytes) ;

    void drawIconAt(Drawing & draw, ITCIcons& icons, SPoint offset, u32 funcIdx, u32 sizeIdx) ;

    ITCStatus() ;

    void setDir6Idx(u32 dir6Idx) ;

    bool isOpen() { return mIsOpen; }
    void setIsOpen(bool newopen) { mIsOpen = newopen; }

    bool isAlive() { return mIsAlive; }
    void setIsAlive(bool newalive) { mIsAlive = newalive; }

    void updateStatsFromLine(ByteSource & bs) ;

    void formatDeltasLine(ByteSink & bs) ;

    void drawStatus(Drawing & draw, ITCIcons & icons) ;
      
  };
}

#endif /* ITCSTATUS_H */
