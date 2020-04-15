#include "T2GridPanel.h"
#include "Point.h"

namespace MFM {

  T2GridPanel::T2GridPanel()
  {
  }

  void T2GridPanel::paintInsane(u32 x, u32 y) {
    DIE_UNIMPLEMENTED();
  }

  void T2GridPanel::paintSite(OurT2Site & s, u32 x, u32 y) {
    OurT2Atom & a = s.GetAtom();
    if (!a.IsSane()) paintInsane(x,y);
    else if (!OurT2Atom::IsType(a,OurT2Atom::ATOM_EMPTY_TYPE)) {
      DIE_UNIMPLEMENTED();
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
      for (u32 i = 0; i < T2TILE_WIDTH; ++i)
        for (u32 j = 0; j < T2TILE_HEIGHT; ++j) 
          paintSite(sites.get(UPoint(i,j)),i,j);
    });
  }
}
