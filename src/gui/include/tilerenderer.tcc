/* -*- C++ -*- */
#include "element_data.h"    /* for ELEMENT_DATA */
#include "element_sorter.h"  /* for ELEMENT_SORTER */
#include "colormap.h"
#include "Util.h"            /* for MIN and MAX */

namespace MFM {

template <class T, u32 R>
u32 TileRenderer::GetAtomColor(Tile<T,R>& tile, const T& atom)
{
  const Element<T,R> * elt = tile.GetElementTable().Lookup(atom.GetType());
  if (elt) return elt->DefaultPhysicsColor();
  return 0xffffffff;
}

template <class T, u32 R>
u32 TileRenderer::GetDataHeatColor(Tile<T,R>& tile, const T& atom)
{
  if(atom.IsType(Element_Sorter<T,R>::TYPE))
  {
    return ColorMap_SEQ5_YlOrRd::THE_INSTANCE.
      GetInterpolatedColor(Element_Sorter<T,R>::THE_INSTANCE.GetThreshold(atom,0),0,100,0xffff0000);
  }
  if(atom.IsType(Element_Data<T,R>::TYPE))
  {
    return ColorMap_SEQ5_YlGnBu::THE_INSTANCE.
      GetInterpolatedColor(Element_Data<T,R>::THE_INSTANCE.GetDatum(atom,0),0,100,0xffff0000);
  }
  return 0;
}


template <class T,u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderAtoms(SPoint& pt, Tile<T,EVENT_WINDOW_RADIUS>& tile,
			       bool renderCache)
{
  u32 astart = renderCache ? 0 : EVENT_WINDOW_RADIUS;
  u32 aend   = renderCache ? TILE_WIDTH : TILE_WIDTH - EVENT_WINDOW_RADIUS;

  u32 cacheOffset = renderCache ? 0 : -EVENT_WINDOW_RADIUS * m_atomDrawSize;

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

	  const T* atom = tile.GetAtom(atomLoc);
	  u32 color;
	  if(m_drawDataHeat)
	  {
	    color = GetDataHeatColor(tile, *atom);
	  }
	  else
	  {
	    color = GetAtomColor(tile, *atom);
	  }


	  if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX() &&
	     rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
	  {
	    if(color)
	    {
	      Drawing::FillCircle(m_dest,
				  rendPt.GetX(),
				  rendPt.GetY(),
				  m_atomDrawSize,
				  m_atomDrawSize,
				  m_atomDrawSize / 2,
				  color);
	    }
	  }
	}
      }
    }
  }
}

template <class T,u32 R>
void TileRenderer::RenderTile(Tile<T,R>& t, SPoint& loc, bool renderWindow,
			      bool renderCache)
{
  SPoint multPt(loc);

  const s32 INTER_CACHE_GAP = 1;
  s32 spacing = renderCache ? TILE_WIDTH + INTER_CACHE_GAP : TILE_WIDTH - R * 2;
  multPt.Multiply(spacing * m_atomDrawSize);

  SPoint realPt(multPt);


  u32 tileHeight = TILE_WIDTH * m_atomDrawSize;

  realPt.Add(m_windowTL);

  if(realPt.GetX() + tileHeight >= 0 &&
     realPt.GetY() + tileHeight >= 0 &&
     realPt.GetX() < (s32)m_dest->w &&
     realPt.GetY() < (s32)m_dest->h)
  {
    if(m_drawMemRegions)
    {
      RenderMemRegions<R>(multPt, renderCache);
    }

    if(renderWindow)
    {
      RenderEventWindow(multPt, t, renderCache);
    }

    RenderAtoms(multPt, t, renderCache);

    if(m_drawGrid)
    {
      RenderGrid<R>(&multPt, renderCache);
    }
  }
}

template <class T,u32 R>
void TileRenderer::RenderEventWindow(SPoint& offset,
				     Tile<T,R>& tile, bool renderCache)
{
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
    ManhattanDir<R>::get().FillFromBits(atomLoc, i, (TableType)R);
    atomLoc.Add(eventCenter);
    atomLoc.Add(cacheOffset, cacheOffset);

    if(i == 0) // Center atom first in indexing.
    {
      drawColor = Drawing::GREEN;
    }
    else
    {
      drawColor = Drawing::WHITE;
    }

    RenderAtomBG(offset, atomLoc, drawColor);
  }
}

template <u32 R>
void TileRenderer::RenderMemRegions(SPoint& pt, bool renderCache)
{
  int regID = 0;
  if(renderCache)
  {
    RenderMemRegion<R>(pt, regID++, m_cacheColor, renderCache);
  }
  RenderMemRegion<R>(pt, regID++, m_sharedColor, renderCache);
  RenderMemRegion<R>(pt, regID++, m_visibleColor, renderCache);
  RenderMemRegion<R>(pt, regID  , m_hiddenColor, renderCache);
}

template <u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderMemRegion(SPoint& pt, int regID,
				   Uint32 color, bool renderCache)
{
  int tileSize;
  if(!renderCache)
  {
    /* Subtract out the cache's width */
    tileSize = m_atomDrawSize *
      (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
  }
  else
  {
    tileSize = m_atomDrawSize * TILE_WIDTH;
  }

  int ewrSize = EVENT_WINDOW_RADIUS * m_atomDrawSize;

  /* Manually fill the rect so we can stop at the right place. */
  Point<s32> topPt(pt.GetX() + (ewrSize * regID) + m_windowTL.GetX(),
		   pt.GetY() + (ewrSize * regID) + m_windowTL.GetY());
  Point<s32> botPt(MIN((s32)m_dimensions.GetX(), topPt.GetX() + (tileSize - (ewrSize * regID * 2))),
		   MIN((s32)m_dimensions.GetY(), topPt.GetY() + (tileSize - (ewrSize * regID * 2))));
  for(s32 x = topPt.GetX(); x < botPt.GetX(); x++)
  {
    for(s32 y = topPt.GetY(); y < botPt.GetY(); y++)
    {
      Drawing::SetPixel(m_dest, x, y, color);
    }
  }
}

template <u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderGrid(SPoint* pt, bool renderCache)
{
  s32 lineLen, linesToDraw;

  if(!renderCache)
  {
    lineLen = m_atomDrawSize *
      (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
    linesToDraw = TILE_WIDTH +
      1 - (2 * EVENT_WINDOW_RADIUS);
  }
  else
  {
    lineLen = m_atomDrawSize * TILE_WIDTH;
    linesToDraw = TILE_WIDTH + 1;
  }

  s32 lowBound =
    MIN((s32)m_dimensions.GetY(), pt->GetY() + m_windowTL.GetY() + lineLen);

  for(int x = 0; x < linesToDraw; x++)
  {
    if(pt->GetX() + x * m_atomDrawSize + m_windowTL.GetX() > (s32)m_dimensions.GetX())
    {
      break;
    }
    Drawing::DrawVLine(m_dest,
		       pt->GetX() + x * m_atomDrawSize +
		       m_windowTL.GetX(),
		       pt->GetY() + m_windowTL.GetY(),
		       lowBound,
		       m_gridColor);
  }

  s32 rightBound =
    MIN((s32)m_dimensions.GetX(), pt->GetX() + m_windowTL.GetX() + lineLen);

  for(int y = 0; y < linesToDraw; y++)
  {
    if(pt->GetY() + y * m_atomDrawSize + m_windowTL.GetY() > (s32)m_dimensions.GetY())
    {
      break;
    }
    Drawing::DrawHLine(m_dest,
		       pt->GetY() + y * m_atomDrawSize +
		       m_windowTL.GetY(),
		       pt->GetX() + m_windowTL.GetX(),
		       rightBound,
		       m_gridColor);
  }
}
} /* namespace MFM */

