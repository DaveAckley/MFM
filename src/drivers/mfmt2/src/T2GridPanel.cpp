#include "T2GridPanel.h"
#include "ChooserPanel.h"
#include "SDLI.h"
#include "Point.h"
#include "DrawableSDL.h" /* for paintCustom to use */

namespace MFM {




  // Handle mouse button action inside our walls
  bool T2GridPanel::Handle(MouseButtonEvent & event)
  {
    // Take default behavior if any keyboard modifiers
    if (event.m_keyboardModifiers) return Super::Handle(event);

    if (event.m_event.type == SDL_MOUSEBUTTONUP)      // Click on up
    {
      const char * name = "GlobalMenu"; 
      T2Tile & tile = T2Tile::get();
      SDLI & sdli = tile.getSDLI();
      ChooserPanel * cp = dynamic_cast<ChooserPanel*>(sdli.lookForPanel(name));
      if (!cp) LOG.Error("Can't find %s",name);
      else{
        cp->toggleGlobalPosting();
        return true;
      }
    }
    return false;
  }

  T2GridPanel::T2GridPanel()
  {
  }

  void T2GridPanel::paintInsane(Drawing & config, u32 x, u32 y) {
    DIE_UNIMPLEMENTED();
  }

  void T2GridPanel::paintSite(Drawing & config, DrawSiteType dtype, DrawSiteShape dshape, OurT2Site & s, u32 x, u32 y) {

    if (dtype == DRAW_SITE_NONE) return; // okay then

    OurT2Atom & a = s.GetAtom();
    if (!a.IsSane()) paintInsane(config, x,y);
    else if (OurT2Atom::IsType(a,OurT2Atom::ATOM_EMPTY_TYPE)) return;

    T2Tile & tile = T2Tile::get();
    UlamEventSystem & ues = tile.getUlamEventSystem();
    u32 atype = a.GetType();
    u32 color = 0xffffffff;
    const OurElement * elt = ues.getElementIfAny(atype);
    if (elt != 0) {
      color = elt->GetDynamicColor(tile.GetElementTable(), tile.GetUlamClassRegistry(), a, 1);
    }
      
    switch (dtype) {
    case DRAW_SITE_ELEMENT:         //< Static color of event layer atom
      if (elt != 0) color = elt->GetStaticColor();
      break;
      
    case DRAW_SITE_ATOM_1:          //< Dynamic per-atom rendering type 1
    case DRAW_SITE_ATOM_2:          //< Dynamic per-atom rendering type 2
      if (elt != 0)
        color = elt->GetDynamicColor(tile.GetElementTable(), tile.GetUlamClassRegistry(), a,
                                     (u32) (dtype - DRAW_SITE_ATOM_1 + 1));
      break;
      
    case DRAW_SITE_BASE:            //< Static color of base atom
    case DRAW_SITE_BASE_1:          //< Dynamic base-atom rendering type 1
    case DRAW_SITE_BASE_2:          //< Dynamic base-atom rendering type 2
    case DRAW_SITE_PAINT:           //< Last color painted on site
      {
        OurT2Base & base = s.GetBase();
        OurT2Atom & baseAtom = base.GetBaseAtom();
        u32 batype = baseAtom.GetType();
        const OurElement * baseelt = ues.getElementIfAny(batype);
        if (baseelt != 0) {
          if (dtype == DRAW_SITE_BASE)
            color = baseelt->GetStaticColor();
          else if (dtype == DRAW_SITE_PAINT)
            color = base.GetPaint();
          else
            color = baseelt->GetDynamicColor(tile.GetElementTable(), tile.GetUlamClassRegistry(), baseAtom,
                                             (u32) (dtype - DRAW_SITE_BASE_1 + 1));
        }
      }
      break;

    case DRAW_SITE_LIGHT_TILE:      //< Light grey rendering of tile regions
    case DRAW_SITE_DARK_TILE:       //< Dark grey rendering of hidden regions
    case DRAW_SITE_CHANGE_AGE:      //< CubeHelix rendering of events-since-change
    case DRAW_SITE_BLACK:           //< Fill with black
      color = 0x0;
      break;
    case DRAW_SITE_DARK:            //< Fill with dark grey
      color = 0xff101010;
      break;
    case DRAW_SITE_WHITE:           //< Fill with white
      color = 0xffffffff;
      break;
    default: break;
    }

#if 0    
    case DRAW_SITE_DARK_TILE:
      {
        Rect r(ditOrigin + ComputeDrawInsetDit(tile, OurTile::REGION_VISIBLE),
               MakeUnsigned(ComputeDrawSizeDit(tile, OurTile::REGION_VISIBLE)));
        drawing.FillRectDit(r, Drawing::GREY20);
      }
      return;

    case DRAW_SITE_LIGHT_TILE:
      {
        for (u32 i = OurTile::REGION_CACHE; i <= OurTile::REGION_HIDDEN; ++i)
        {
          if (!IsRegionDrawn(tile, i)) continue;

          Rect r(ditOrigin + ComputeDrawInsetDit(tile, i),
                 MakeUnsigned(ComputeDrawSizeDit(tile, i)));
          drawing.FillRectDit(r, m_regionColors[i]);
        }
      }
      return;
#endif

      const SPoint ditPos(T2_SITE_TO_DIT_X*x, T2_SITE_TO_DIT_Y*y);
      const UPoint ditSiz(T2_SITE_TO_DIT_W,T2_SITE_TO_DIT_H);
      const Rect ditRect(ditPos, ditSiz);


      switch (dshape) {
      case DRAW_SHAPE_FILL:           //< Flood fill site entirely (square)
        config.FillRectDit(ditRect,color);
        break;
      case DRAW_SHAPE_CIRCLE:         //< Draw circle touching site edges
        config.FillCircleDit(ditRect,ditSiz.GetX()/2,color);
        break;
      case DRAW_SHAPE_CDOT:           //< Draw small centered dot
        {
          UPoint cdotSiz(T2_SITE_TO_DIT_W/4u,T2_SITE_TO_DIT_H/4u);
          SPoint inset = MakeSigned(ditSiz - cdotSiz) / 2;
          Rect r(ditPos + inset, cdotSiz);
          config.FillRectDit(r, color);
        }
        break;
      default: break;
      }
    }

    static const Rect makeBorderDitRect(Dir6 dir6) {
    Rect ret;
    u32 ditWid = 2*T2_DISPLAY_DIT_PER_PIX;
    switch (dir6) {
    default: LOG.Error("%s Illegal dir6 %d", __PRETTY_FUNCTION__, dir6); //break;
    case DIR6_ET:
      ret.SetPosition(SPoint(T2_SITE_TO_DIT_X*T2TILE_OWNED_WIDTH-ditWid/2,0));
      ret.SetSize(UPoint(ditWid,T2_SITE_TO_DIT_H*T2TILE_OWNED_HEIGHT));
      break;
    case DIR6_SE:
      ret.SetPosition(SPoint(T2_SITE_TO_DIT_X*T2TILE_OWNED_WIDTH/2,
                             T2_SITE_TO_DIT_Y*T2TILE_OWNED_HEIGHT-ditWid/2));
      ret.SetSize(UPoint(T2_SITE_TO_DIT_W*T2TILE_OWNED_WIDTH/2,ditWid));
      break;
    case DIR6_SW:
      ret.SetPosition(SPoint(0,
                             T2_SITE_TO_DIT_Y*T2TILE_OWNED_HEIGHT-ditWid/2));
      ret.SetSize(UPoint(T2_SITE_TO_DIT_W*T2TILE_OWNED_WIDTH/2,ditWid));
      break;
    case DIR6_WT:
      ret.SetPosition(SPoint(0,0));
      ret.SetSize(UPoint(ditWid,T2_SITE_TO_DIT_H*T2TILE_OWNED_HEIGHT));
      break;
    case DIR6_NW:
      ret.SetPosition(SPoint(0,0));
      ret.SetSize(UPoint(T2_SITE_TO_DIT_W*T2TILE_OWNED_WIDTH/2,ditWid));
      break;
    case DIR6_NE:
      ret.SetPosition(SPoint(T2_SITE_TO_DIT_X*T2TILE_OWNED_WIDTH/2,0));
      ret.SetSize(UPoint(T2_SITE_TO_DIT_W*T2TILE_OWNED_WIDTH/2,ditWid));
      break;

    }
    return ret;
  }

  void T2GridPanel::PaintBorder(Drawing & drawing)
  {
    T2Tile & tile = T2Tile::get();
    KITCPoller & kitc = tile.getKITCPoller();
    for (Dir6 dir6 = 0; dir6 < DIR6_COUNT; ++dir6) {
      const Rect borderDitRect = makeBorderDitRect(dir6);
      T2ITC & itc = tile.getITC(dir6);
      u32 kitcEnabled = kitc.getKITCEnabledStatus(mapDir6ToDir8(dir6));
      ITCStateNumber sn = itc.getITCSN();
      u32 color = Drawing::WHITE;
      if (kitcEnabled == 0) color= Drawing::RED;
      else if (kitcEnabled == 1) color = Drawing::ORANGE;
      else if (sn==ITCSN_SHUT) color = Drawing::YELLOW;
      else if (sn==ITCSN_OPEN) color = Drawing::BLACK; // Don't draw
      drawing.FillRectDit(borderDitRect, color);
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
    const SiteRenderConfig & src = tile.getSiteRenderConfig();
    unwind_protect({
      LOG.Warning("Paint failed at (%d,%d); grid render incomplete", i, j);
      return;
    },{
        for (u32 i = CACHE_LINES; i < T2TILE_WIDTH-CACHE_LINES; ++i) {
          for (u32 j = CACHE_LINES; j < T2TILE_HEIGHT-CACHE_LINES; ++j) {
            OurT2Site & site = sites.get(UPoint(i,j));
            paintSite(config, src.mBackType, DRAW_SHAPE_FILL,
                      site, i-CACHE_LINES, j-CACHE_LINES);
            if (src.mMidType != src.mBackType)
              paintSite(config, src.mMidType, DRAW_SHAPE_CIRCLE,
                        site, i-CACHE_LINES, j-CACHE_LINES);
            if (src.mFrontType != src.mMidType)
              paintSite(config, src.mFrontType, DRAW_SHAPE_CDOT,
                        site, i-CACHE_LINES, j-CACHE_LINES);
          }
        }
      });
    

    if (src.mCustomGraphics) paintCustomTile(config);
  }

  void T2GridPanel::paintCustomTile(Drawing & config) {
    typedef UlamContextEventSDL<OurT2EventConfig> OurUlamContextEventSDL;
    typedef EventWindowRendererSDL<OurT2EventConfig> OurEventWindowRendererSDL;

    //    LOG.Message("In T2GridPanel::paintCustomTile(Drawing & config) ");

    T2Tile & tile = T2Tile::get();
    Sites & sites = tile.getSites();
    //    UlamEventSystem & ues = tile.getUlamEventSystem();

    DrawableSDL drawable(config);
    OurEventWindowRendererSDL ewrs(drawable);
    OurUlamContextEventSDL ucs(ewrs, tile); // NB: No longer UlamContextRestrictedSDL!

    OurEventWindow & ew = tile.GetEventWindow();

    const u32 maxditsiz =
      (T2_SITE_TO_DIT_W > T2_SITE_TO_DIT_H ? T2_SITE_TO_DIT_W : T2_SITE_TO_DIT_H);

    for (u32 i = CACHE_LINES; i < T2TILE_WIDTH-CACHE_LINES; ++i) {
      for (u32 j = CACHE_LINES; j < T2TILE_HEIGHT-CACHE_LINES; ++j) {
        SPoint sitecoord(i,j);
        UPoint usitecoord(i,j);
        UPoint viscoord(i-CACHE_LINES,j-CACHE_LINES);

        OurT2Site & site = sites.get(usitecoord);
        OurT2Atom & a = site.GetAtom();

        if (!a.IsSane()) continue;

        u32 type = a.GetType();
        if (type == OurT2Atom::ATOM_EMPTY_TYPE) continue;

        //        LOG.Message("paintCustomTile(%x)",type);

        OurAtomBitStorage abs(a);

        const OurElement * elt = tile.GetElementTable().Lookup(type);
        if (!elt) continue;

        const OurUlamElement * uelt = elt->AsUlamElement();
        if (!uelt) continue;

        bool DEBUGPRINT = false; //strcmp("H3",uelt->GetAtomicSymbol());
        if (DEBUGPRINT)
          LOG.Message("paintCustomTile('%s'@(%d,%d))",
                      uelt->GetAtomicSymbol(),
                      sitecoord.GetX(), sitecoord.GetY());

        if (ew.InitForEvent(sitecoord, false)) {

          const SPoint ditPos(T2_SITE_TO_DIT_X*viscoord.GetX() + T2_SITE_TO_DIT_X/2,
                              T2_SITE_TO_DIT_Y*viscoord.GetY() + T2_SITE_TO_DIT_Y/2);

          drawable.Reset();
          drawable.SetDitOrigin(ditPos);
          drawable.SetDitsPerSite(maxditsiz);

          if (DEBUGPRINT)
            LOG.Message("paintCustomTile:ditpos(%d,%d)ditsiz(%d)",
                        ditPos.GetX(),ditPos.GetY(),maxditsiz);

          unwind_protect({
              const char * failFile = MFMThrownFromFile;
              const unsigned lineno = MFMThrownFromLineNo;
              const char * failMsg = MFMFailCodeReason(MFMThrownFailCode);
              OString256 buff;
              SPointSerializer ssp(sitecoord);
              buff.Printf("%@: Render failed: %s (%s:%d)",
                          &ssp,
                          failMsg,
                          failFile,
                          lineno);
              LOG.Message("%s",buff.GetZString());
            },
            {
              CallRenderGraphics(ucs,*uelt,abs);
            });
          ew.SetFree();
        }
      }
    }
  }

  void T2GridPanel::CallRenderGraphics(OurUlamContextEvent & ucs,
                                       const OurUlamElement & uelt,
                                       OurAtomBitStorage & abs)
  {

    //    const UlamClassRegistry<EC>& ucr = tile.GetUlamClassRegistry();

    OurUlamRef ur(OurT2Atom::ATOM_FIRST_STATE_BIT, uelt.GetClassLength(), abs, &uelt, OurUlamRef::ELEMENTAL, ucs);
    // how to do an ulam virtual function call in c++
    VfuncPtr vfuncptr;
    OurUlamRef vfur(ur, OurUlamElement::RENDERGRAPHICS_VOWNED_INDEX, 0u, true, vfuncptr);
    typedef void (* Uf_9214renderGraphics)(const OurUlamContext& uc, OurUlamRef& ur);
    ((Uf_9214renderGraphics) vfuncptr) (ucs, vfur);
    //    LOG.Message("CallRenderGraphics done");
  }

#if 0
  void T2GridPanel::paintCustom(Drawing & config, UPoint siteInTileCoord, OurT2Site & s, u32 x, u32 y) {

    
    OurT2Atom & a = s.GetAtom();

    if (!a.IsSane()) return;

    u32 atype = a.GetType();
    if (atype == OurT2Atom::ATOM_EMPTY_TYPE) return;

    const OurElement * elt = ues.getElementIfAny(atype);
    if (elt != 0) return;

    const OurUlamElement * uelt = elt->AsUlamElement();
    if (uelt != 0) return;

    if (ew.InitForEvent(siteInTileCoord, false)) {
      drawable.Reset();
      drawable.SetDitOrigin(siteOriginDit);
      drawable.SetDitsPerSite(m_atomSizeDit);
      
      CallRenderGraphics(ucs,*uelt,abs,tile);
    }
  }
#endif

}
  
