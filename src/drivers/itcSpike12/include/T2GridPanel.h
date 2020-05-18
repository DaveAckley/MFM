/* -*- C++ -*- */
#ifndef T2GRIDPANEL_H
#define T2GRIDPANEL_H

#include "itype.h"
#include "Panel.h"
#include "T2Tile.h"

namespace MFM {
  struct T2GridPanel : public Panel {
    
    T2GridPanel() ;
    virtual ~T2GridPanel() { }

    virtual void PaintComponent(Drawing & config) ;

    void paintSite(Drawing & config, OurT2Site & s, u32 atx, u32 aty) ;
    void paintInsane(Drawing & config, u32 atx, u32 aty) ;
  };
}
#endif /* T2GRIDPANEL_H */
