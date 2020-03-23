/* -*- C++ -*- */
#ifndef ITCSTATUSPANEL_H
#define ITCSTATUSPANEL_H

#include "Panel.h"
#include "ITC.h"
#include "dirdatamacro.h"

namespace MFM {
  class ITCIcons; // FORWARD
  
  struct ITCStatusPanel : public Panel {

    static const char * (dirNames[ITC_COUNT]);
#if 0
    static const u32 (iconFaceIdx[ITC_COUNT]);
#endif
    static const UPoint (screenPos[ITC_COUNT]);
    static const SPoint (side1Pos[ITC_COUNT]);
    static const SPoint (side2Pos[ITC_COUNT]);

    void init(ITC & itc, ITCIcons & icons) ;
    ITC * mITC;
    ITCIcons * mIcons;
    ITC & getITC() {
      MFM_API_ASSERT_NONNULL(mITC);
      return *mITC;
    }
    const ITC & getITC() const {
      MFM_API_ASSERT_NONNULL(mITC);
      return *mITC;
    }
    ITCIcons & getITCIcons() {
      MFM_API_ASSERT_NONNULL(mIcons);
      return *mIcons;
    }

    const u32 getWidth() const ;
    const u32 getHeight() const ;

    const bool isVertical() const ;

    virtual void PaintComponent(Drawing & config) ;
  };

}

#endif /* ITCSTATUSPANEL_H */
