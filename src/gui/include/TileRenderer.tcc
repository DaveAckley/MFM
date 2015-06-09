/* -*- C++ -*- */
#include "Util.h"            /* for MIN and MAX */
#include "ColorMap.h"        /* for CubeHelix */

namespace MFM
{

#if 0
  template <class EC>
  u32 TileRenderer::GetForegroundColor(Tile<EC>& tile, const Site<typename EC::ATOM_CONFIG> & site,
                                 u32 selector)
  {
    const typename EC::ATOM_CONFIG::ATOM_TYPE & atom = site.GetAtom();
    const Element<EC> * elt = tile.GetElementTable().Lookup(atom.GetType());
    if (elt)
    {
      return elt->LocalPhysicsColor(site,selector);
    }
    return 0xffffffff;
  }
#endif

#if 0
  template <class EC>
  u32 TileRenderer::GetDataHeatColor(Tile<EC>& tile, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom)
  {
    return GetAtomColor(tile,atom,1);
  }
#endif


  template <class EC>
  void TileRenderer::RenderAtoms(Drawing & drawing, SPoint& pt, Tile<EC>& tile,
                                 bool renderCache, bool lowlight)
  {
    u32 astart = renderCache ? 0 : EC::EVENT_WINDOW_RADIUS;
    u32 aend   = renderCache ? tile.TILE_SIDE : tile.TILE_SIDE - EC::EVENT_WINDOW_RADIUS;

    u32 cacheOffset = renderCache ? 0 : -EC::EVENT_WINDOW_RADIUS * m_atomDrawSize;

    SPoint atomLoc;

    Point<u32> rendPt;

    for(u32 x = astart; x < aend; x++)
    {
      rendPt.SetX(pt.GetX() + m_atomDrawSize * x +
                  m_windowTL.GetX() + cacheOffset);
      if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX())
      {
        atomLoc.SetX(x);
        for(u32 y = astart; y < aend; y++)
        {
          rendPt.SetY(pt.GetY() + m_atomDrawSize * y +
                      m_windowTL.GetY() + cacheOffset);
          if(rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
          {
            atomLoc.SetY(y);
            if (m_drawBackgroundType == DRAW_BACKGROUND_CHANGE_AGE && tile.IsOwnedSite(atomLoc))
            {
              // Draw background 'write heat' map
              u32 writeAge = tile.GetUncachedWriteAge32(atomLoc -
                                                        SPoint(EC::EVENT_WINDOW_RADIUS,
                                                               EC::EVENT_WINDOW_RADIUS));
              u32 colorIndex = 0;
              const u32 MAX_IDX = 10000;       // Potential (interpolated) colors
              const u32 AGE_PER_AEPS = tile.GetSites();
              const double MAX_EXPT = 4.0;     // 10**4.0 == 10kAEPS for fully black
              const double LOG_SCALER = MAX_IDX/MAX_EXPT;
              double writeAgeAEPS = 1.0 * writeAge / AGE_PER_AEPS + 1;

              colorIndex = MIN(MAX_IDX, (u32) (LOG_SCALER*log10(writeAgeAEPS)));
              u32 color =
                ColorMap_CubeHelixRev::THE_INSTANCE.
                GetInterpolatedColor(colorIndex,0,MAX_IDX,0xffff0000);

              drawing.SetForeground(color);
              drawing.FillRect(rendPt.GetX(),
                               rendPt.GetY(),
                               m_atomDrawSize,
                               m_atomDrawSize);
            }
            else if (m_drawBackgroundType == DRAW_BACKGROUND_SITE && tile.IsOwnedSite(atomLoc))
            {
              typedef typename EC::ATOM_CONFIG AC;
              const Site<AC> & site = tile.GetSite(atomLoc);

              drawing.SetForeground(site.GetPaint());
              drawing.FillRect(rendPt.GetX(),
                               rendPt.GetY(),
                               m_atomDrawSize,
                               m_atomDrawSize);
            }
            RenderAtom(drawing, atomLoc, rendPt, tile, lowlight);
          }
        }
      }
    }
  }


  template <class EC>
  void TileRenderer::RenderBadAtom(Drawing& drawing, const UPoint& rendPt)
  {
    for(s32 x = 0; x < (s32)m_atomDrawSize; x++)
    {
      for(s32 y = 0; y < (s32)m_atomDrawSize; y++)
      {
        if((x + y + 1) & 4)
        {
          drawing.SetForeground(Drawing::YELLOW);
        }
        else
        {
          drawing.SetForeground(Drawing::BLACK);
        }
        drawing.FillRect(rendPt.GetX() + x,
                         rendPt.GetY() + y, 1, 1);
      }
    }
  }

  template <class EC>
  void TileRenderer::RenderAtom(Drawing & drawing, const SPoint& atomLoc,
                                const UPoint& rendPt,  Tile<EC>& tile, bool lowlight)
  {

    if (m_drawForegroundType == DRAW_FOREGROUND_NONE) return;

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    const Site<AC> & site = tile.GetSite(atomLoc);

    u32 color;
    const char * elementLabel = 0;
    const u32 LABEL_ATOM_SIZE = 36;

    if (m_drawForegroundType == DRAW_FOREGROUND_SITE)
    {
      color = site.GetPaint();
    }
    else
    {

      const T & atom = site.GetAtom();
      if(!atom.IsSane())
      {
        RenderBadAtom<EC>(drawing, rendPt);
        return;
      }

      u32 type = atom.GetType();

      if (type == T::ATOM_EMPTY_TYPE) return;

      const Element<EC> * elt = tile.GetElementTable().Lookup(type);
      if (!elt)
      {
        RenderBadAtom<EC>(drawing, rendPt);
        return;
      }

      if (m_atomDrawSize >= LABEL_ATOM_SIZE)
      {
        elementLabel = elt->GetAtomicSymbol();
      }

      if (m_drawForegroundType == DRAW_FOREGROUND_ELEMENT)
      {
        color = elt->PhysicsColor();
      }
      else if (m_drawForegroundType >= DRAW_FOREGROUND_ATOM_1 &&
               m_drawForegroundType <= DRAW_FOREGROUND_ATOM_3)
      {
        color = elt->LocalPhysicsColor(site,m_drawForegroundType - DRAW_FOREGROUND_ATOM_1 + 1);
      }
      else
        FAIL(ILLEGAL_STATE);

      if(lowlight)
      {
        color = Drawing::HalfColor(color);
      }
    }

    if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX() &&
       rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
    {
      {
        // Round up on radius.  Better to overlap than vanish
        u32 radius = (m_atomDrawSize + 1) / 2;

        drawing.SetForeground(color);
        if(m_renderSquares)
        {
          drawing.FillRect(rendPt.GetX(),
                           rendPt.GetY(),
                           m_atomDrawSize,
                           m_atomDrawSize);
        }
        else
        {
          drawing.FillCircle(rendPt.GetX(),
                             rendPt.GetY(),
                             m_atomDrawSize,
                             m_atomDrawSize,
                             radius);
        }

        if (elementLabel)
        {
          drawing.SetFont(FONT_ASSET_ELEMENT);
          const SPoint size = drawing.GetTextSize(elementLabel);
          const UPoint box = UPoint(m_atomDrawSize, m_atomDrawSize);
          if (size.GetX() > 0 && size.GetY() > 0)
          {
            const UPoint usize(size.GetX(), size.GetY());
            drawing.SetBackground(Drawing::BLACK);
            drawing.SetForeground(Drawing::WHITE);
            drawing.BlitBackedTextCentered(elementLabel, rendPt, box);
          }
        }
      }
    }
  }

  template <class EC>
  void TileRenderer::RenderTile(Drawing & drawing, Tile<EC>& t, SPoint& loc, bool renderWindow,
                                bool renderCache, bool selected, SPoint* selectedAtom,
                                SPoint* cloneOrigin)
  {
    const u32 TILE_SIDE = t.TILE_SIDE;

    SPoint multPt(loc);

    const s32 INTER_CACHE_GAP = 1;
    s32 spacing = renderCache ?
                  TILE_SIDE + INTER_CACHE_GAP :
                  TILE_SIDE - EC::EVENT_WINDOW_RADIUS * 2;

    multPt.Multiply(spacing * m_atomDrawSize);

    SPoint realPt(multPt);


    u32 tileHeight = TILE_SIDE * m_atomDrawSize;

    bool lowlight = !t.IsActive();

    realPt.Add(m_windowTL);

    if(realPt.GetX() + tileHeight >= 0 &&
       realPt.GetY() + tileHeight >= 0)
    {
      switch (m_drawBackgroundType)
      {
      default:
      case DRAW_BACKGROUND_SITE: // Handled in RenderAtoms
        // FALL THROUGH

      case DRAW_BACKGROUND_CHANGE_AGE: // Handled in RenderAtoms
        // FALL THROUGH

      case DRAW_BACKGROUND_NONE:
        break;

      case DRAW_BACKGROUND_LIGHT_TILE:
        RenderMemRegions<EC>(drawing, multPt, renderCache, selected, lowlight, TILE_SIDE);
        break;

      case DRAW_BACKGROUND_DARK_TILE:
        RenderVisibleRegionOutlines<EC>(drawing, multPt, renderCache, selected, lowlight, TILE_SIDE);
        break;
      }

      if(renderWindow)
      {
        RenderEventWindow(drawing, multPt, t, renderCache);
      }

      RenderAtoms(drawing, multPt, t, renderCache, lowlight);

      if(m_drawGrid)
      {
        RenderGrid<EC>(drawing, &multPt, renderCache, TILE_SIDE);
      }

      if(selectedAtom)
      {
        SPoint ap = *selectedAtom * m_atomDrawSize + realPt;

        drawing.SetForeground(Drawing::YELLOW);
        drawing.DrawRectangle(Rect(ap, UPoint(m_atomDrawSize, m_atomDrawSize)));
      }

      if(cloneOrigin)
      {
        SPoint cp = *cloneOrigin * m_atomDrawSize + realPt;

        drawing.SetForeground(Drawing::CYAN);
        drawing.DrawRectangle(Rect(cp, UPoint(m_atomDrawSize, m_atomDrawSize)));
      }
    }
  }

  template <class EC>
  void TileRenderer::RenderEventWindow(Drawing & drawing, SPoint& offset,
                                       Tile<EC>& tile, bool renderCache)
  {

#if 1 // Sun Oct 19 11:39:41 2014 Unreimplemented since not currently in use
    // Extract short type names
    enum { R = EC::EVENT_WINDOW_RADIUS};

    SPoint atomLoc;
    SPoint eventCenter;
    u32 cacheOffset = renderCache ? 0 : -R;
    u32 drawColor = Drawing::WHITE;

    tile.FillLastExecutedAtom(eventCenter);
    u32 tableSize = EVENT_WINDOW_SITES(R);
    for(u32 i = 0; i < tableSize; i++)
    {
      MDist<R>::get().FillFromBits(atomLoc, i, R);
      atomLoc.Add(eventCenter);
      atomLoc.Add(cacheOffset, cacheOffset);

      if(i == 0)  // Center atom first in indexing.
      {
        drawColor = Drawing::GREEN;
      }
      else
      {
        drawColor = Drawing::WHITE;
      }

      RenderAtomBG(drawing, offset, atomLoc, drawColor);
    }
#endif
  }

  template <class EC>
  void TileRenderer::RenderMemRegions(Drawing & drawing, SPoint& pt,
                                      bool renderCache, bool selected, bool lowlight,
                                      const u32 TILE_SIDE)
  {
    // Extract short type names
    enum { R = EC::EVENT_WINDOW_RADIUS};

    int regID = 0;
    if(renderCache)
    {
      RenderMemRegion<EC>(drawing, pt, regID++, !lowlight ?
                          m_cacheColor : Drawing::HalfColor(m_cacheColor), renderCache,
                          TILE_SIDE);
    }
    RenderMemRegion<EC>(drawing, pt, regID++, !lowlight ?
                        m_sharedColor : Drawing::HalfColor(m_sharedColor), renderCache,
                        TILE_SIDE);
    RenderMemRegion<EC>(drawing, pt, regID++, !lowlight ?
                        m_visibleColor : Drawing::HalfColor(m_visibleColor), renderCache,
                        TILE_SIDE);
    RenderMemRegion<EC>(drawing, pt, regID,
                        selected ? m_selectedHiddenColor :
                                   (!lowlight ?
                                    m_hiddenColor : Drawing::HalfColor(m_hiddenColor)),
                        renderCache,
                        TILE_SIDE);
  }

  template <class EC>
  void TileRenderer::RenderVisibleRegionOutlines(Drawing & drawing, SPoint& pt,
                                                 bool renderCache, bool selected, bool lowlight,
                                                 const u32 TILE_SIDE)
  {
    u32 regID = renderCache ? 2 : 1;

    if(!lowlight)
    {
      RenderMemRegion<EC>(drawing, pt, regID,
                          selected ? 0xff606060 : 0xff202020, renderCache,
                          TILE_SIDE);
    }
    else
    {
      RenderMemRegion<EC>(drawing, pt, regID,
                          selected ? 0xff303030 : 0xff101010, renderCache,
                          TILE_SIDE);
    }
  }

  template <class EC>
  void TileRenderer::RenderMemRegion(Drawing & drawing, SPoint& pt, int regID,
                                     Uint32 color, bool renderCache,
                                     const u32 TILE_SIDE)
  {
    int tileSize;
    if(!renderCache)
    {
      /* Subtract out the cache's width */
      tileSize = m_atomDrawSize *
        (TILE_SIDE - 2 * EC::EVENT_WINDOW_RADIUS);
    }
    else
    {
      tileSize = m_atomDrawSize * TILE_SIDE;
    }

    int ewrSize = EC::EVENT_WINDOW_RADIUS * m_atomDrawSize;

    /* Find rectangle to fill. */
    SPoint topPt(pt.GetX() + (ewrSize * regID) + m_windowTL.GetX(),
                 pt.GetY() + (ewrSize * regID) + m_windowTL.GetY());
    SPoint botPt(MIN((s32)m_dimensions.GetX(), topPt.GetX() + (tileSize - (ewrSize * regID * 2))),
                 MIN((s32)m_dimensions.GetY(), topPt.GetY() + (tileSize - (ewrSize * regID * 2))));

    drawing.FillRect(topPt.GetX(), topPt.GetY(),
                     botPt.GetX() - topPt.GetX(),
                     botPt.GetY() - topPt.GetY(),
                     color);
  }

  template <class EC>
  void TileRenderer::RenderGrid(Drawing & drawing, SPoint* pt, bool renderCache, const u32 TILE_SIDE)
  {
    s32 lineLen, linesToDraw;

    if(!renderCache)
    {
      lineLen = m_atomDrawSize *
        (TILE_SIDE - 2 * EC::EVENT_WINDOW_RADIUS);
      linesToDraw = TILE_SIDE +
        1 - (2 * EC::EVENT_WINDOW_RADIUS);
    }
    else
    {
      lineLen = m_atomDrawSize * TILE_SIDE;
      linesToDraw = TILE_SIDE + 1;
    }

    s32 lowBound =
      MIN((s32)m_dimensions.GetY(), pt->GetY() + m_windowTL.GetY() + lineLen);

    drawing.SetForeground(m_gridColor);
    for(int x = 0; x < linesToDraw; x++)
    {
      drawing.DrawVLine(pt->GetX() + x * m_atomDrawSize +
                        m_windowTL.GetX(),
                        pt->GetY() + m_windowTL.GetY(),
                        lowBound);
    }

    s32 rightBound =
      MIN((s32)m_dimensions.GetX(), pt->GetX() + m_windowTL.GetX() + lineLen);

    for(int y = 0; y < linesToDraw; y++)
    {
      drawing.DrawHLine(pt->GetY() + y * m_atomDrawSize +
                        m_windowTL.GetY(),
                        pt->GetX() + m_windowTL.GetX(),
                        rightBound);
    }
  }
} /* namespace MFM */
