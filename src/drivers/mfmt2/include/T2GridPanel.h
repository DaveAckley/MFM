/* -*- C++ -*- */
#ifndef T2GRIDPANEL_H
#define T2GRIDPANEL_H

#include "itype.h"
#include "Panel.h"
#include "T2Tile.h"

namespace MFM {
  struct T2GridPanel : public Panel {
    typedef Panel Super;

    // Handle mouse button action inside our walls
    virtual bool Handle(MouseButtonEvent & event) ;

    T2GridPanel() ;
    virtual ~T2GridPanel() { }

    // Make border sections reflect ITC status
    void PaintBorder(Drawing & drawing) ;

    virtual void PaintComponent(Drawing & config) ;

    void paintSite(Drawing & config, DrawSiteType dtype, DrawSiteShape dshape, OurT2Site & s, u32 x, u32 y) ;

    void paintCustomTile(Drawing & config) ;

    void CallRenderGraphics(OurUlamContextEvent & ucs,
                            const OurUlamElement & uelt,
                            OurAtomBitStorage & abs) ;

    void paintInsane(Drawing & config, u32 atx, u32 aty) ;
  };
}
#endif /* T2GRIDPANEL_H */
