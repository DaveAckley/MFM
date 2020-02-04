#include "ITCStatusPanel.h"
#include "ITCIcons.h"

namespace MFM {
  const char * (ITCStatusPanel::dirNames[ITC_COUNT]) =
    { "ET", "SE", "SW", "WT", "NW", "NE" };

#if 0 // ??
  const u32 (ITCStatusPanel::iconFaceIdx[ITC_COUNT]) =
  { ITCIcons::ICON_DIR_NORTH_IDX,
    ITCIcons::ICON_DIR_EAST_IDX,
    ITCIcons::ICON_DIR_SOUTH_IDX,
    ITCIcons::ICON_DIR_SOUTH_IDX,
    ITCIcons::ICON_DIR_WEST_IDX,
    ITCIcons::ICON_DIR_NORTH_IDX
  };
#endif

  enum {
    HALF_WIDTH = ROOT_WINDOW_WIDTH/2,
    HALF_HEIGHT = ROOT_WINDOW_HEIGHT/2,
    ICON_SIDE = 32,
    STATUS_PIX_IN = ICON_SIDE,
    STATUS_PIX_OVER = 2*ICON_SIDE,
    WIDER = 60
  };

  const UPoint (ITCStatusPanel::screenPos[ITC_COUNT]) = {
    UPoint(ROOT_WINDOW_WIDTH-STATUS_PIX_IN, (ROOT_WINDOW_HEIGHT-STATUS_PIX_OVER)/2),           // ET
    UPoint(HALF_WIDTH+(HALF_WIDTH-STATUS_PIX_OVER)/2+WIDER, ROOT_WINDOW_HEIGHT-STATUS_PIX_IN), // SE
    UPoint((HALF_WIDTH-STATUS_PIX_OVER)/2-WIDER, ROOT_WINDOW_HEIGHT-STATUS_PIX_IN),            // SW
    UPoint(0, (ROOT_WINDOW_HEIGHT-STATUS_PIX_OVER)/2),                                         // WT
    UPoint((HALF_WIDTH-STATUS_PIX_OVER)/2-WIDER, 0),                                            // NW
    UPoint(HALF_WIDTH+(HALF_WIDTH-STATUS_PIX_OVER)/2+WIDER, 0),                                // NE
  };

  // side 1 relative to render point
  const SPoint (ITCStatusPanel::side1Pos[ITC_COUNT]) = {
    SPoint(0, 0),              // ET
    SPoint(ICON_SIDE,0),       // SE
    SPoint(ICON_SIDE,0),       // SW
    SPoint(0, ICON_SIDE),      // WT
    SPoint(0, 0),               // NW
    SPoint(0, 0),              // NE
  };

  // side 2 relative to render point
  const SPoint (ITCStatusPanel::side2Pos[ITC_COUNT]) = {
    SPoint(0, ICON_SIDE),      // ET
    SPoint(0, 0),              // SE
    SPoint(0, 0),              // SW
    SPoint(0, 0),              // WT
    SPoint(ICON_SIDE, 0),       // NW
    SPoint(ICON_SIDE, 0),      // NE
  };

  const u32 ITCStatusPanel::getWidth() const {  return isVertical() ? ITC_IN_PIXELS : ITC_ACROSS_PIXELS; }
  const u32 ITCStatusPanel::getHeight() const { return isVertical() ? ITC_ACROSS_PIXELS : ITC_IN_PIXELS; }
  const bool ITCStatusPanel::isVertical() const 
  {
    return getITC().getDir6() == DIR_ET || getITC().getDir6() == DIR_WT;
  }

  void ITCStatusPanel::init(ITC & itc, ITCIcons & icons) {
    mITC = &itc;
    mIcons = &icons;
    SetName(dirNames[getITC().getDir6()]);
    SetVisible(true);
    s32 w = getWidth(), h = getHeight();
    SetDimensions(w,h);
    SetDesiredSize(w,h);
    SPoint pos(MakeSigned(screenPos[getITC().getDir6()]));
    SetRenderPoint(pos);
    SetForeground(Drawing::YELLOW);
    SetBackground(Drawing::BLACK);
    SetBorder(Drawing::BLACK);
  }

  void ITCStatusPanel::PaintComponent(Drawing & draw) {
    MFM_API_ASSERT_STATE(mITC != 0 && mIcons != 0);
    const ITC & itc = getITC();
    u32 level = itc.getLevel();
    u32 color = Drawing::MAGENTA;
    if (level == 0) {
      if (itc.getStage() == 0) {
        if (itc.getEnabled()) color = Drawing::RED;
        else color = Drawing::GREY10;
      }
      else
        color = Drawing::BLUE;
    } else if (level == 1) {
      if (itc.getKnownIncompatible()) color = Drawing::RED;
      else color = Drawing::GREEN;
    } else if (level > 1)
      color = Drawing::YELLOW;

    draw.SetBackground(color);
    draw.Clear();
  }
}
