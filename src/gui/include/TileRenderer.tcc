/* -*- C++ -*- */
#include "Util.h"            /* for MIN and MAX */

namespace MFM {

template <class CC>
u32 TileRenderer::GetAtomColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom, u32 selector)
{
  const Element<CC> * elt = tile.GetElementTable().Lookup(atom.GetType());
  if (elt)
  {
    return elt->LocalPhysicsColor(atom,selector);
  }
  return 0xffffffff;
}

template <class CC>
u32 TileRenderer::GetDataHeatColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom)
{
  return GetAtomColor(tile,atom,1);
}


template <class CC>
void TileRenderer::RenderAtoms(Drawing & drawing, SPoint& pt, Tile<CC>& tile,
                               bool renderCache, bool lowlight)
{
  // Extract short type names
  typedef typename CC::PARAM_CONFIG P;

  u32 astart = renderCache ? 0 : P::EVENT_WINDOW_RADIUS;
  u32 aend   = renderCache ? P::TILE_WIDTH : P::TILE_WIDTH - P::EVENT_WINDOW_RADIUS;

  u32 cacheOffset = renderCache ? 0 : -P::EVENT_WINDOW_RADIUS * m_atomDrawSize;

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

          if(tile.GetAtom(atomLoc)->IsSane())
          {
            RenderAtom(drawing, atomLoc, rendPt, tile, lowlight);
          }
          else
          {
            for(s32 x = 0; x < (s32) m_atomDrawSize; x++)
            {
              for(s32 y = 0; y < (s32) m_atomDrawSize; y++)
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
        }
      }
    }
  }
}

  template <class CC>
  void TileRenderer::RenderAtom(Drawing & drawing, const SPoint& atomLoc, const UPoint& rendPt,  Tile<CC>& tile, bool lowlight)
{
  const typename CC::ATOM_TYPE * atom = tile.GetAtom(atomLoc);
  if(atom->GetType() != Element_Empty<CC>::THE_INSTANCE.GetType())
  {
    u32 color;
    if(m_drawDataHeat)
    {
      color = GetDataHeatColor(tile, *atom);
    }
    else
    {
      color = GetAtomColor(tile, *atom);
    }

    if(lowlight)
    {
      color = Drawing::HalfColor(color);
    }

    if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX() &&
       rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
    {
      if(color)
      {
        // Round up on radius.  Better to overlap than vanish
        u32 radius = (m_atomDrawSize + 1) / 2;

        drawing.SetForeground(color);
        drawing.FillCircle(rendPt.GetX(),
                           rendPt.GetY(),
                           m_atomDrawSize,
                           m_atomDrawSize,
                           radius);

        if (m_atomDrawSize > 40)
        {
          const Element<CC> * elt = tile.GetElement(atom->GetType());
          if (elt)
          {
            drawing.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
            const char * sym = elt->GetAtomicSymbol();
            const SPoint size = drawing.GetTextSize(sym);
            const UPoint box = UPoint(m_atomDrawSize, m_atomDrawSize);
            if (size.GetX() > 0 && size.GetY() > 0)
            {
              const UPoint usize(size.GetX(), size.GetY());
              drawing.SetBackground(Drawing::BLACK);
              drawing.SetForeground(Drawing::WHITE);
              drawing.BlitBackedTextCentered(sym, rendPt, box);
            }
          }
        }
      }
    }
  }
}

  template <class CC>
  void TileRenderer::RenderTile(Drawing & drawing, Tile<CC>& t, SPoint& loc, bool renderWindow,
                                bool renderCache, bool selected, SPoint* selectedAtom,
                                SPoint* cloneOrigin)
  {
    // Extract short type names
    typedef typename CC::PARAM_CONFIG P;

    SPoint multPt(loc);

    const s32 INTER_CACHE_GAP = 1;
    s32 spacing = renderCache ?
                  P::TILE_WIDTH + INTER_CACHE_GAP :
                  P::TILE_WIDTH - P::EVENT_WINDOW_RADIUS * 2;

    multPt.Multiply(spacing * m_atomDrawSize);

    SPoint realPt(multPt);


    u32 tileHeight = P::TILE_WIDTH * m_atomDrawSize;

    bool lowlight = !t.GetExecutingOwnEvents();

    realPt.Add(m_windowTL);

    if(realPt.GetX() + tileHeight >= 0 &&
       realPt.GetY() + tileHeight >= 0 /* &&  Go ahead and draw, we'll clip?
                                          realPt.GetX() < (s32)m_dest->w &&
                                          realPt.GetY() < (s32)m_dest->h*/)
    {
      switch (m_drawMemRegions)
      {
      default:
      case NO:
        break;
      case FULL:
        RenderMemRegions<CC>(drawing, multPt, renderCache, selected, lowlight);
        break;
      case EDGE:
        RenderVisibleRegionOutlines<CC>(drawing, multPt, renderCache, selected, lowlight);
        break;
      }

      if(renderWindow)
      {
        RenderEventWindow(drawing, multPt, t, renderCache);
      }

      RenderAtoms(drawing, multPt, t, renderCache, lowlight);

      if(m_drawGrid)
      {
        RenderGrid<CC>(drawing, &multPt, renderCache);
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

  template <class CC>
  void TileRenderer::RenderEventWindow(Drawing & drawing, SPoint& offset, Tile<CC>& tile, bool renderCache)
  {
    // Extract short type names
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS};

    SPoint winCenter;
    tile.FillLastExecutedAtom(winCenter);

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
  }

  template <class CC>
  void TileRenderer::RenderMemRegions(Drawing & drawing, SPoint& pt,
                                      bool renderCache, bool selected, bool lowlight)
  {
    // Extract short type names
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS};

    int regID = 0;
    if(renderCache)
    {
      RenderMemRegion<CC>(drawing, pt, regID++, !lowlight ?
                          m_cacheColor : Drawing::HalfColor(m_cacheColor), renderCache);
    }
    RenderMemRegion<CC>(drawing, pt, regID++, !lowlight ?
                          m_sharedColor : Drawing::HalfColor(m_sharedColor), renderCache);
    RenderMemRegion<CC>(drawing, pt, regID++, !lowlight ?
                          m_visibleColor : Drawing::HalfColor(m_visibleColor), renderCache);
    RenderMemRegion<CC>(drawing, pt, regID,
                        selected ? m_selectedHiddenColor :
                                   (!lowlight ?
                                    m_hiddenColor : Drawing::HalfColor(m_hiddenColor)),
                        renderCache);
  }

  template <class CC>
  void TileRenderer::RenderVisibleRegionOutlines(Drawing & drawing, SPoint& pt,
                                                 bool renderCache, bool selected, bool lowlight)
  {
    u32 regID = renderCache ? 2 : 1;


    if(!lowlight)
    {
      RenderMemRegion<CC>(drawing, pt, regID,
                          selected ? 0xff606060 : 0xff202020, renderCache);
    }
    else
    {
      RenderMemRegion<CC>(drawing, pt, regID,
                          selected ? 0xff303030 : 0xff101010, renderCache);
    }
  }

  template <class CC>
  void TileRenderer::RenderMemRegion(Drawing & drawing, SPoint& pt, int regID,
                                     Uint32 color, bool renderCache)
  {
    // Extract short names for parameter types
    typedef typename CC::PARAM_CONFIG P;

    int tileSize;
    if(!renderCache)
    {
      /* Subtract out the cache's width */
      tileSize = m_atomDrawSize *
        (P::TILE_WIDTH - 2 * P::EVENT_WINDOW_RADIUS);
    }
    else
    {
      tileSize = m_atomDrawSize * P::TILE_WIDTH;
    }

    int ewrSize = P::EVENT_WINDOW_RADIUS * m_atomDrawSize;

    /* Find rectangle to fill. */
    Point<s32> topPt(pt.GetX() + (ewrSize * regID) + m_windowTL.GetX(),
                     pt.GetY() + (ewrSize * regID) + m_windowTL.GetY());
    Point<s32> botPt(MIN((s32)m_dimensions.GetX(), topPt.GetX() + (tileSize - (ewrSize * regID * 2))),
                     MIN((s32)m_dimensions.GetY(), topPt.GetY() + (tileSize - (ewrSize * regID * 2))));

    drawing.FillRect(topPt.GetX(), topPt.GetY(),
                     botPt.GetX() - topPt.GetX(),
                     botPt.GetY() - topPt.GetY(),
                     color);
  }

  template <class CC>
  void TileRenderer::RenderGrid(Drawing & drawing, SPoint* pt, bool renderCache)
  {
// Extract short type names
    typedef typename CC::PARAM_CONFIG P;
    s32 lineLen, linesToDraw;

    if(!renderCache)
    {
      lineLen = m_atomDrawSize *
        (P::TILE_WIDTH - 2 * P::EVENT_WINDOW_RADIUS);
      linesToDraw = P::TILE_WIDTH +
        1 - (2 * P::EVENT_WINDOW_RADIUS);
    }
    else
    {
      lineLen = m_atomDrawSize * P::TILE_WIDTH;
      linesToDraw = P::TILE_WIDTH + 1;
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
