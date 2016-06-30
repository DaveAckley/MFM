/* -*- C++ -*- */
#include "Util.h"            /* for MIN and MAX */
#include "ColorMap.h"        /* for CubeHelix */

namespace MFM
{
  template <class EC>
  TileRenderer<EC>::TileRenderer()
    : m_drawBackgroundType(DRAW_SITE_DARK_TILE)
    , m_drawMidgroundType(DRAW_SITE_ATOM_1)
    , m_drawForegroundType(DRAW_SITE_NONE)
    , m_drawEventWindow(false)
    , m_drawGridLines(true)
    , m_drawCacheSites(true)
    , m_drawBases(false)
    , m_atomSizeDit(DEFAULT_ATOM_SIZE_DIT)
    , m_gridLineColor(Drawing::GREY30)
  {
    m_regionColors[OurTile::REGION_CACHE] = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 100);
    m_regionColors[OurTile::REGION_SHARED] = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 92);
    m_regionColors[OurTile::REGION_VISIBLE] = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 84);
    m_regionColors[OurTile::REGION_HIDDEN] = InterpolateColors(Drawing::WHITE, Drawing::DARK_PURPLE, 76);
  }

  template <class EC>
  SPoint TileRenderer<EC>::ComputeDrawSizeDit(const Tile<EC> & tile, u32 tileRegion) const
  {
    const u32 drawDiameterSites = tile.TILE_SIDE - 2 * EWR * tileRegion;
    const u32 drawSideDit = drawDiameterSites * m_atomSizeDit;
    return SPoint(drawSideDit, drawSideDit);
  }

  template <class EC>
  void TileRenderer<EC>::PaintSites(Drawing & drawing,
                                    const DrawSiteType drawType,
                                    const DrawSiteShape shape,
                                    const SPoint ditOrigin,
                                    const Tile<EC> & tile)
  {
    // First deal with the whole-tile cases
    switch (drawType)
    {
    default: break;  // Deal with illegal or non-floodable below

    case DRAW_SITE_NONE:
      // That was easy
      return;

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
    }

    // Here we need to iterate over the sites
    typename OurTile::const_iterator_type end = tile.end(m_drawCacheSites);
    for (typename OurTile::const_iterator_type i = tile.begin(m_drawCacheSites); i != end; ++i)
    {
      SPoint siteInTileCoord = i.At();
      SPoint screenDitForSite = ditOrigin + siteInTileCoord * m_atomSizeDit;
      PaintSiteAtDit(drawing, drawType, shape, screenDitForSite, *i, tile);
    }
  }


  template <class EC>
  void TileRenderer<EC>::PaintUnderlays(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile)
  {
    if (m_drawEventWindow)
      FAIL(INCOMPLETE_CODE);
  }

  template <class EC>
  void TileRenderer<EC>::PaintOverlays(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile)
  {
    if (m_drawGridLines)
    {
      u32 outerRegion = m_drawCacheSites ? OurTile::REGION_CACHE : OurTile::REGION_SHARED;
      SPoint tileSizeDit = ComputeDrawSizeDit(tile, outerRegion);

      for (s32 ditOff = 0; ditOff <= tileSizeDit.GetX(); ditOff += m_atomSizeDit)
      {
        drawing.DrawHLineDit(ditOrigin.GetY() + ditOff,
                             ditOrigin.GetX(), ditOrigin.GetX() + tileSizeDit.GetX(),
                             m_gridLineColor);
        drawing.DrawVLineDit(ditOrigin.GetX() + ditOff,
                             ditOrigin.GetY(), ditOrigin.GetY() + tileSizeDit.GetY(),
                             m_gridLineColor);
      }
    }
  }

  template <class EC>
  void TileRenderer<EC>::PaintTileAtDit(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile)
  {
    if (!tile.IsEnabled())
    {
        Rect rv(ditOrigin + ComputeDrawInsetDit(tile, OurTile::REGION_VISIBLE),
                MakeUnsigned(ComputeDrawSizeDit(tile, OurTile::REGION_VISIBLE)));
        drawing.FillRectDit(rv, Drawing::GREY10);

        return;
    }
    PaintSites(drawing,
               (m_drawBases && !IsBaseVisible()) ? DRAW_SITE_BASE : m_drawBackgroundType,
               DRAW_SHAPE_FILL, ditOrigin, tile);
    PaintUnderlays(drawing, ditOrigin, tile);  // E.g. an event window

    PaintSites(drawing, m_drawMidgroundType, DRAW_SHAPE_CIRCLE, ditOrigin, tile);

    PaintSites(drawing, m_drawForegroundType, DRAW_SHAPE_CDOT, ditOrigin, tile);

    PaintOverlays(drawing, ditOrigin, tile);   // E.g., a tool footprint
  }

  template <class EC>
  void TileRenderer<EC>::OutlineEventWindowInTile(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile, SPoint site, u32 color)
  {
    if (!m_drawCacheSites) site -= SPoint(EWR, EWR);
    const MDist<EWR> & md = MDist<EWR>::get();
    for (u32 s = md.GetFirstIndex(EWR); s <= md.GetLastIndex(EWR); ++s)
    {
      SPoint rel = md.GetPoint(s);
      SPoint abs = site + rel;
      SPoint ulDit = ditOrigin + abs * m_atomSizeDit;
      if (rel.GetX() <= 0) 
        drawing.DrawVLineDit(ulDit.GetX(),ulDit.GetY(), ulDit.GetY() + m_atomSizeDit, color);
      if (rel.GetX() >= 0) 
        drawing.DrawVLineDit(ulDit.GetX() + m_atomSizeDit, ulDit.GetY(), ulDit.GetY() + m_atomSizeDit, color);
      if (rel.GetY() <= 0) 
        drawing.DrawHLineDit(ulDit.GetY(), ulDit.GetX(), ulDit.GetX() + m_atomSizeDit, color);
      if (rel.GetY() >= 0) 
        drawing.DrawHLineDit(ulDit.GetY() + m_atomSizeDit, ulDit.GetX(), ulDit.GetX() + m_atomSizeDit, color);
    }
  }

  template <class EC>
  void TileRenderer<EC>::PaintTileHistoryInfo(Drawing & drawing, const SPoint ditOrigin, const Tile<EC> & tile) 
  {
    const EventHistoryBuffer<EC> & ehb = tile.GetEventHistoryBuffer();
    if (!ehb.IsHistoryActive()) return;
    bool atEnd = ehb.IsCursorAtAnEventEnd();
    bool atStart = ehb.IsCursorAtAnEventStart();
    if (!atEnd && !atStart) return;
    SPoint ctr;
    if (!ehb.SiteOfCursor(ctr)) return;
    OutlineEventWindowInTile(drawing, ditOrigin, tile, ctr, atEnd ? Drawing::RED : Drawing::GREEN);
  }

  template <class EC>
  void TileRenderer<EC>::PaintSiteAtDit(Drawing & drawing,
                                        const DrawSiteType drawType,
                                        const DrawSiteShape shape,
                                        const SPoint ditOrigin,
                                        const Site<AC> & site,
                                        const Tile<EC> & inTile)
  {
    u32 selector = 0;
    bool fromBase = false;
    switch (drawType)
    {
    default:
      FAIL(ILLEGAL_STATE);

    case DRAW_SITE_NONE:
    case DRAW_SITE_DARK_TILE:
    case DRAW_SITE_LIGHT_TILE:
      // Flat backgrounds were handled per-tile..
      return;

    case DRAW_SITE_CHANGE_AGE:
      {
        const u32 writeAge = site.GetWriteAge();
        const u32 MAX_IDX = 10000;       // Potential (interpolated) colors
        const u32 AGE_PER_AEPS = 1; // Counting site events directly.., was: tile.GetSites();
        const double MAX_EXPT = 4.0;     // 10**4.0 == 10kAEPS for fully black
        const double LOG_SCALER = MAX_IDX/MAX_EXPT;
        const double writeAgeAEPS = 1.0 * writeAge / AGE_PER_AEPS + 1;
        const u32 colorIndex = MIN(MAX_IDX, (u32) (LOG_SCALER*log10(writeAgeAEPS)));
        const u32 color =
          ColorMap_CubeHelixRev::THE_INSTANCE.
          GetInterpolatedColor(colorIndex,0,MAX_IDX,0xffff0000);

        PaintShapeForSite(drawing, shape, ditOrigin, color);
      }
      return;

    case DRAW_SITE_PAINT:
      {
        PaintShapeForSite(drawing, shape, ditOrigin, site.GetPaint());
      }
      return;

    case DRAW_SITE_ELEMENT: break;
    case DRAW_SITE_ATOM_1: selector = 1; break;
    case DRAW_SITE_ATOM_2: selector = 2; break;

    case DRAW_SITE_BASE: fromBase = true; break;
    case DRAW_SITE_BASE_1: fromBase = true; selector = 1; break;
    case DRAW_SITE_BASE_2: fromBase = true; selector = 2; break;

    }

    // Here if we need to do atom-specific painting

    u32 drawColor;
    const char * elementLabel  = 0;

    const T & atom = fromBase ? site.GetBase().GetBaseAtom() : site.GetAtom();
    if(!atom.IsSane())
    {
      // XXX HANDLE INSANE SHAPE?
      PaintBadAtomAtDit(drawing, ditOrigin);
      return;
    }

    u32 type = atom.GetType();

    if (type == T::ATOM_EMPTY_TYPE) return;

    const Element<EC> * elt = inTile.GetElementTable().Lookup(type);
    if (!elt)
    {
      // XXX HANDLE INSANE SHAPE? DITTO
      PaintBadAtomAtDit(drawing, ditOrigin);
      return;
    }

    const u32 LABEL_ATOM_SIZE_DIT = Drawing::MapPixToDit(50);
    if (m_atomSizeDit >= LABEL_ATOM_SIZE_DIT)
    {
      elementLabel = elt->GetAtomicSymbol();
    }

    if (selector == 0)
    {
      drawColor = elt->GetStaticColor();
    }
    else
    {
      drawColor = elt->GetDynamicColor(inTile.GetElementTable(), atom, selector);
    }
    PaintShapeForSite(drawing, shape, ditOrigin, drawColor);

    const u32 atomDit = m_atomSizeDit;

    UPoint pixSize = Drawing::MapDitToPix(UPoint(atomDit, atomDit));
    SPoint pixOrigin = Drawing::MapDitToPix(ditOrigin);

    if (elementLabel && shape == DRAW_SHAPE_CIRCLE)
    {
      drawing.SetFont(FONT_ASSET_ELEMENT);
      const SPoint size = drawing.GetTextSize(elementLabel);

      drawing.SetBackground(Drawing::BLACK);
      drawing.SetForeground(Drawing::WHITE);
      drawing.BlitBackedTextCentered(elementLabel, pixOrigin, pixSize);
    }

  }

  template <class EC>
  void TileRenderer<EC>::PaintShapeForSite(Drawing & drawing, const DrawSiteShape shape, const SPoint ditOrigin, u32 color)
  {
    switch (shape)
    {
    default: FAIL(INCOMPLETE_CODE);
    case DRAW_SHAPE_CIRCLE:
      {
        Rect r(ditOrigin, UPoint(m_atomSizeDit, m_atomSizeDit));
        drawing.FillCircleDit(r, m_atomSizeDit/2, color);
      }
      break;
    case DRAW_SHAPE_FILL:
      {
        Rect r(ditOrigin, UPoint(m_atomSizeDit, m_atomSizeDit));
        drawing.FillRectDit(r, color);
      }
      break;
    case DRAW_SHAPE_CDOT:
      {
        u32 dotDit = m_atomSizeDit / 5;
        if (dotDit < Drawing::DIT_PER_PIX) dotDit = Drawing::DIT_PER_PIX;
        UPoint fullSize(m_atomSizeDit, m_atomSizeDit);
        UPoint size(dotDit,dotDit);
        SPoint inset = MakeSigned(fullSize - size) / 2;
        Rect r(ditOrigin + inset, size);
        drawing.FillRectDit(r, color);
      }
      break;
    }
  }

  template <class EC>
  void TileRenderer<EC>::PaintBadAtomAtDit(Drawing & drawing, const SPoint ditOrigin)
  {
    Rect r(Drawing::MapDitToPix(ditOrigin), Drawing::MapDitToPix(UPoint(m_atomSizeDit, m_atomSizeDit)));
    drawing.BlitImageAssetTiled(IMAGE_ASSET_BAD_ATOM_TILE, r);
  }

  template <class EC>
  const char * TileRenderer<EC>::GetDrawSiteTypeName(DrawSiteType t)
  {
    switch (t)
    {
    default:
      FAIL(ILLEGAL_ARGUMENT);
    case DRAW_SITE_ELEMENT:       return "Element";
    case DRAW_SITE_ATOM_1:        return "Atom #1";
    case DRAW_SITE_ATOM_2:        return "Atom #2";
    case DRAW_SITE_BASE:          return "Base";
    case DRAW_SITE_BASE_1:        return "Base #1";
    case DRAW_SITE_BASE_2:        return "Base #2";
    case DRAW_SITE_LIGHT_TILE:    return "Light tile";
    case DRAW_SITE_DARK_TILE:     return "Dark tile";
    case DRAW_SITE_CHANGE_AGE:    return "Change age";
    case DRAW_SITE_PAINT:         return "Site paint";
    case DRAW_SITE_NONE:          return "None";
    }
  }

  template <class EC>
  void TileRenderer<EC>::TileRendererSaveDetails(ByteSink & sink) const
  {
    sink.Printf(" PP(traz=%d)\n", m_atomSizeDit);
    sink.Printf(" PP(trbt=%d)\n", m_drawBackgroundType);
    sink.Printf(" PP(trdc=%d)\n", m_drawCacheSites);
    sink.Printf(" PP(trdg=%d)\n", m_drawGridLines);
    sink.Printf(" PP(trev=%d)\n", m_drawEventWindow);
    sink.Printf(" PP(trmt=%d)\n", m_drawMidgroundType);
    sink.Printf(" PP(trft=%d)\n", m_drawForegroundType);
    sink.Printf(" PP(trgc=%d)\n", m_gridLineColor);
  }

  template <class EC>
  bool TileRenderer<EC>::TileRendererLoadDetails(const char * key, LineCountingByteSource & source)
  {
    if (!strcmp("traz",key)) return 1 == source.Scanf("%?d", sizeof m_atomSizeDit, &m_atomSizeDit);
    if (!strcmp("trbt",key)) return 1 == source.Scanf("%?d", sizeof m_drawBackgroundType, &m_drawBackgroundType);
    if (!strcmp("trdc",key)) return 1 == source.Scanf("%?d", sizeof m_drawCacheSites, &m_drawCacheSites);
    if (!strcmp("trdg",key)) return 1 == source.Scanf("%?d", sizeof m_drawGridLines, &m_drawGridLines);
    if (!strcmp("trev",key)) return 1 == source.Scanf("%?d", sizeof m_drawEventWindow, &m_drawEventWindow);
    if (!strcmp("trmt",key)) return 1 == source.Scanf("%?d", sizeof m_drawMidgroundType, &m_drawMidgroundType);
    if (!strcmp("trft",key)) return 1 == source.Scanf("%?d", sizeof m_drawForegroundType, &m_drawForegroundType);
    if (!strcmp("trgc",key)) return 1 == source.Scanf("%?d", sizeof m_gridLineColor, &m_gridLineColor);

    return false;
  }

} /* namespace MFM */
