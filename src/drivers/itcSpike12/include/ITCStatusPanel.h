/* -*- C++ -*- */
#ifndef ITCSTATUSPANEL_H
#define ITCSTATUSPANEL_H

#include "Panel.h"

namespace MFM {
  class ITCStatus; // FORWARD
  class ITCIcons; // FORWARD
  
  struct ITCStatusPanel : public Panel {
    void init(ITCStatus & status, ITCIcons & icons) ;
    ITCStatus * mStatus;
    ITCIcons * mIcons;

    const u32 getWidth() const ;
    const u32 getHeight() const ;

    virtual void PaintComponent(Drawing & config) ;
  };

}

#endif /* ITCSTATUSPANEL_H */
