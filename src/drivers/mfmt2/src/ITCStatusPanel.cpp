#include "ITCStatusPanel.h"
#include "ITCStatus.h"
#include "ITCIcons.h"

namespace MFM {
  const u32 ITCStatusPanel::getWidth() const {  return mStatus->isVertical() ? ITC_IN_PIXELS : ITC_ACROSS_PIXELS; }
  const u32 ITCStatusPanel::getHeight() const { return mStatus->isVertical() ? ITC_ACROSS_PIXELS : ITC_IN_PIXELS; }

  void ITCStatusPanel::init(ITCStatus & status, ITCIcons & icons) {
    mStatus = &status;
    mIcons = &icons;
    SetName(status.getAbbr());
    SetVisible(true);
    s32 w = getWidth(), h = getHeight();
    SetDimensions(w,h);
    SetDesiredSize(w,h);
    SPoint pos(MakeSigned(mStatus->getScreenPos()));
    SetRenderPoint(pos);
    SetForeground(Drawing::YELLOW);
    SetBackground(Drawing::BLACK);
    SetBorder(Drawing::BLACK);
  }

  void ITCStatusPanel::PaintComponent(Drawing & draw) {
    MFM_API_ASSERT_STATE(mStatus != 0 && mIcons != 0);
    mStatus->drawStatus(draw, *mIcons);
  }
}
