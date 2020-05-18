#include "T2GridPanel.h"
#include "Point.h"

namespace MFM {

  T2GridPanel::T2GridPanel()
  {
  }

  void T2GridPanel::paintInsane(Drawing & config, u32 x, u32 y) {
    DIE_UNIMPLEMENTED();
  }

  void T2GridPanel::paintSite(Drawing & config, OurT2Site & s, u32 x, u32 y) {
    OurT2Atom & a = s.GetAtom();
    if (!a.IsSane()) paintInsane(config, x,y);
    else if (!OurT2Atom::IsType(a,OurT2Atom::ATOM_EMPTY_TYPE)) {
      u32 atype = a.GetType();
      u32 color;
      switch (atype) {
      case T2_PHONY_DREG_TYPE: color = 0xff444444; break;
      case T2_PHONY_RES_TYPE: color = 0xff999944; break;
      default: color = 0xffffffff; break;
      }

      const SPoint ditPos(T2_SITE_TO_DIT_X*x, T2_SITE_TO_DIT_Y*y);
      const UPoint ditSiz(T2_SITE_TO_DIT_W,T2_SITE_TO_DIT_H);
      const Rect ditRect(ditPos, ditSiz);
      config.FillCircleDit(ditRect,ditSiz.GetX()/2,color);
    }
  }

  void T2GridPanel::PaintComponent(Drawing & config) {
    this->Panel::PaintComponent(config);
    T2Tile & tile = T2Tile::get();
    Sites & sites = tile.getSites();
    u32 i = U32_MAX, j = U32_MAX;
    /* We're hoping failures during painting will be exceedingly rare
       and so blowing the rest of the render when they happen, rather
       than paying the setjmp price for each site individually. */
    unwind_protect({
      LOG.Warning("Paint failed at (%d,%d); grid render incomplete", i, j);
    },{
      for (u32 i = CACHE_LINES; i < T2TILE_WIDTH-CACHE_LINES; ++i)
        for (u32 j = CACHE_LINES; j < T2TILE_HEIGHT-CACHE_LINES; ++j) 
          paintSite(config,sites.get(UPoint(i,j)),
                    i-CACHE_LINES,
                    j-CACHE_LINES);
    });
  }
}
