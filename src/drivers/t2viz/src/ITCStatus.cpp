
#include "Logger.h"
#include "ITCStatus.h"
#include "ITCIcons.h"

namespace MFM {
  const char * (ITCStatus::dirNames[ITC_COUNT]) =
  { "NE", "ET", "SE", "SW", "WT", "NW" };

  const u32 (ITCStatus::iconFaceIdx[ITC_COUNT]) =
  { ITCIcons::ICON_DIR_NORTH_IDX,
    ITCIcons::ICON_DIR_EAST_IDX,
    ITCIcons::ICON_DIR_SOUTH_IDX,
    ITCIcons::ICON_DIR_SOUTH_IDX,
    ITCIcons::ICON_DIR_WEST_IDX,
    ITCIcons::ICON_DIR_NORTH_IDX
  };

  enum {
    HALF_WIDTH = ROOT_WINDOW_WIDTH/2,
    HALF_HEIGHT = ROOT_WINDOW_HEIGHT/2,
    ICON_SIDE = 32,
    STATUS_PIX_IN = ICON_SIDE,
    STATUS_PIX_OVER = 2*ICON_SIDE,
    WIDER = 60
  };

  const UPoint (ITCStatus::screenPos[ITC_COUNT]) = {
    UPoint(HALF_WIDTH+(HALF_WIDTH-STATUS_PIX_OVER)/2+WIDER, 0),                                // NE
    UPoint(ROOT_WINDOW_WIDTH-STATUS_PIX_IN, (ROOT_WINDOW_HEIGHT-STATUS_PIX_OVER)/2),           // ET
    UPoint(HALF_WIDTH+(HALF_WIDTH-STATUS_PIX_OVER)/2+WIDER, ROOT_WINDOW_HEIGHT-STATUS_PIX_IN), // SE
    UPoint((HALF_WIDTH-STATUS_PIX_OVER)/2-WIDER, ROOT_WINDOW_HEIGHT-STATUS_PIX_IN),            // SW
    UPoint(0, (ROOT_WINDOW_HEIGHT-STATUS_PIX_OVER)/2),                                         // WT
    UPoint((HALF_WIDTH-STATUS_PIX_OVER)/2-WIDER, 0)                                            // NW
  };

  // side 1 relative to render point
  const SPoint (ITCStatus::side1Pos[ITC_COUNT]) = {
    SPoint(0, 0),              // NE
    SPoint(0, 0),              // ET
    SPoint(ICON_SIDE,0),       // SE
    SPoint(ICON_SIDE,0),       // SW
    SPoint(0, ICON_SIDE),      // WT
    SPoint(0, 0)               // NW
  };

  // side 2 relative to render point
  const SPoint (ITCStatus::side2Pos[ITC_COUNT]) = {
    SPoint(ICON_SIDE, 0),      // NE
    SPoint(0, ICON_SIDE),      // ET
    SPoint(0, 0),              // SE
    SPoint(0, 0),              // SW
    SPoint(0, 0),              // WT
    SPoint(ICON_SIDE, 0)       // NW
  };

  void ITCStatus::setDirIdx(u32 diridx) {
    MFM_API_ASSERT_ARG(diridx < ITC_COUNT);
    mDirIdx = diridx;
  }

  ITCStatus::ITCStatus()
    : mDirIdx(U32_MAX)
    , mIsOpen(false)
  {
    for (u32 i = 0; i < ITC_STAT_COUNT; ++i) {
      mStats[i] = 0;
      mDeltas[i] = U32_MAX; // flag that we're uninitted
    }
  }

  void ITCStatus::updateStatsFromLine(ByteSource & bs) {
    // Discard dir field
    if (bs.Scanf("%d ",0) < 2) { LOG.Error("Bad dir format"); return; }
    for (u32 i = 0; i < ITC_STAT_COUNT; ++i) {
      u32 num;
      if (bs.Scanf("%d",&num) != 1) { LOG.Error("Bad field format"); return; }
      if (mDeltas[i] == U32_MAX) mDeltas[i] = 0;
      else mDeltas[i] = num - mStats[i];
      mStats[i] = num;
    }
   bs.SkipSet("[\n]");         // Eat newline
  }

  const char * ITCStatus::getAbbr() const {
    if (mDirIdx >= ITC_COUNT) return "??";
    return dirNames[mDirIdx];
  }
    
  void ITCStatus::formatDeltasLine(ByteSink & bs) {
    bs.Printf("%s",getAbbr());
    if (mDeltas[BYTES_SENT_IDX] > 0) bs.Printf(" %d>%d",mDeltas[BYTES_SENT_IDX],mDeltas[PACKETS_SENT_IDX]);
    if (mDeltas[BYTES_RCVD_IDX] > 0) bs.Printf(" %d<%d",mDeltas[BYTES_RCVD_IDX],mDeltas[PACKETS_RCVD_IDX]);
    if (mDeltas[SYNC_FAILS_IDX] > 0) bs.Printf(" -sn");
    if (mDeltas[PACKET_SYNCS_IDX] > 0) bs.Printf(" +sn");
    if (mDeltas[TIMEOUTS_IDX] > 0) bs.Printf(" tm");
    bs.Printf("\n");
  }

  void ITCStatus::drawStatus(Drawing & draw, ITCIcons & icons) {
    u32 size1Idx = ITCIcons::ICON_SIZE_IDX_COUNT; // Assume full size
    u32 size2Idx = ITCIcons::ICON_SIZE_IDX_COUNT; // Assume full size
    u32 func1Idx = ITCIcons::ICON_FUNC_EMPTY_IDX;
    u32 func2Idx = ITCIcons::ICON_FUNC_EMPTY_IDX;
    if (mDeltas[SYNC_FAILS_IDX] > 0 || mDeltas[TIMEOUTS_IDX] > 0) {
      func1Idx = func2Idx = ITCIcons::ICON_FUNC_ERROR_IDX;
    } else if (mDeltas[BYTES_SENT_IDX] > 0 || mDeltas[BYTES_RCVD_IDX] > 0) {
      if (mDeltas[BYTES_SENT_IDX] > 0) {
        func1Idx = ITCIcons::ICON_FUNC_SEND_IDX;
        size1Idx = mapBytesToSizeIdx(mDeltas[BYTES_SENT_IDX]);
      } 
      if (mDeltas[BYTES_RCVD_IDX] > 0) {
        func2Idx = ITCIcons::ICON_FUNC_RECV_IDX;
        size2Idx = mapBytesToSizeIdx(mDeltas[BYTES_RCVD_IDX]);
      }
    } else if (!mIsOpen) {
      func1Idx = func2Idx = ITCIcons::ICON_FUNC_CLOSED_IDX;
    }

    drawIconAt(draw, icons, side1Pos[mDirIdx], func1Idx, size1Idx);
    drawIconAt(draw, icons, side2Pos[mDirIdx], func2Idx, size2Idx);
  }

  void ITCStatus::drawIconAt(Drawing & draw, ITCIcons& icons, SPoint offset, u32 funcIdx, u32 sizeIdx) {
    UPoint srcpos, srcsize;
    if (!icons.getIconPosAndSize(iconFaceIdx[mDirIdx], funcIdx, sizeIdx, srcpos, srcsize)) {
      LOG.Error("Can't draw func %d, size %d", funcIdx, sizeIdx);
      return;
    }
    SPoint dstpos= (SPoint(ITC_IN_PIXELS,ITC_IN_PIXELS) - MakeSigned(srcsize))/2;  // Indent by half the unused size
    dstpos += offset;
    Rect srcRect(MakeSigned(srcpos), srcsize);
    draw.BlitSubImage(icons.getZSheet(), srcRect, dstpos);
  }

  u32 ITCStatus::mapBytesToSizeIdx(u32 bytes) {
    if (bytes <=    0) return 0;
    if (bytes <=    1) return 1;
    if (bytes <=    2) return 2;
    if (bytes <=    4) return 3;
    if (bytes <=    8) return 4;
    if (bytes <=   16) return 5;
    if (bytes <=   32) return 6;
    if (bytes <=   64) return 7;
    if (bytes <=  128) return 8;
    if (bytes <=  256) return 9;
    if (bytes <=  512) return 10;
    if (bytes <= 1024) return 11;
    if (bytes <= 2048) return 12;
    if (bytes <= 4096) return 13;
    if (bytes <= 8192) return 14;
    return 15;
  }

}
