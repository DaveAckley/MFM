/* -*- C++ -*- */
#if 0
#include "Element_Data.h"    /* for ELEMENT_DATA */
#include "Element_Sorter.h"  /* for ELEMENT_SORTER */
#endif
#include "colormap.h"
#include "Util.h"            /* for MIN and MAX */

namespace MFM {

template <class CC>
u32 TileRenderer::GetAtomColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom)
{
  const Element<CC> * elt = tile.GetElementTable().Lookup(atom.GetType());
  if (elt) return elt->LocalPhysicsColor(atom);
  return 0xffffffff;
}

template <class CC>
u32 TileRenderer::GetDataHeatColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom)
{
#if 0
  if(Atom<CC>::IsType(atom,Element_Sorter<CC>::TYPE))
  {
    return ColorMap_SEQ5_YlOrRd::THE_INSTANCE.
      GetInterpolatedColor(Element_Sorter<CC>::THE_INSTANCE.GetThreshold(atom,0),0,100,0xffff0000);
  }
  if(Atom<CC>::IsType(atom,Element_Data<CC>::TYPE))
  {
    return ColorMap_SEQ5_YlGnBu::THE_INSTANCE.
      GetInterpolatedColor(Element_Data<CC>::THE_INSTANCE.GetDatum(atom,0),0,100,0xffff0000);
  }
#endif
  return GetAtomColor(tile,atom);
}


template <class CC>
void TileRenderer::RenderAtoms(SPoint& pt, Tile<CC>& tile, bool renderCache)
{
  // Extract short type names
  typedef typename CC::ATOM_TYPE T;
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

	  const typename CC::ATOM_TYPE * atom = tile.GetAtom(atomLoc);
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

template <class CC>
void TileRenderer::RenderTile(Tile<CC>& t, SPoint& loc, bool renderWindow,
			      bool renderCache, bool selected)
{
  // Extract short type names
  typedef typename CC::ATOM_TYPE T;
  typedef typename CC::PARAM_CONFIG P;

  SPoint multPt(loc);

  const s32 INTER_CACHE_GAP = 1;
  s32 spacing = renderCache ? P::TILE_WIDTH + INTER_CACHE_GAP : P::TILE_WIDTH - P::EVENT_WINDOW_RADIUS * 2;
  multPt.Multiply(spacing * m_atomDrawSize);

  SPoint realPt(multPt);


  u32 tileHeight = P::TILE_WIDTH * m_atomDrawSize;

  realPt.Add(m_windowTL);

  if(realPt.GetX() + tileHeight >= 0 &&
     realPt.GetY() + tileHeight >= 0 &&
     realPt.GetX() < (s32)m_dest->w &&
     realPt.GetY() < (s32)m_dest->h)
  {
    switch (m_drawMemRegions) {
    default:
    case NO:
      break;
    case FULL:
      RenderMemRegions<CC>(multPt, renderCache, selected);
      break;
    case EDGE:
      RenderVisibleRegionOutlines<CC>(multPt, renderCache);
      break;
    }

    if(renderWindow)
    {
      RenderEventWindow(multPt, t, renderCache);
    }

    RenderAtoms(multPt, t, renderCache);

    if(m_drawGrid)
    {
      RenderGrid<CC>(&multPt, renderCache);
    }
  }
}

template <class CC>
void TileRenderer::RenderEventWindow(SPoint& offset, Tile<CC>& tile, bool renderCache)
{
  // Extract short type names
  typedef typename CC::ATOM_TYPE T;
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

template <class CC>
void TileRenderer::RenderMemRegions(SPoint& pt, bool renderCache, bool selected)
{
  // Extract short type names
  typedef typename CC::ATOM_TYPE T;
  typedef typename CC::PARAM_CONFIG P;
  enum { R = P::EVENT_WINDOW_RADIUS};

  int regID = 0;
  if(renderCache)
  {
    RenderMemRegion<CC>(pt, regID++, m_cacheColor, renderCache);
  }
  RenderMemRegion<CC>(pt, regID++, m_sharedColor, renderCache);
  RenderMemRegion<CC>(pt, regID++, m_visibleColor, renderCache);
  RenderMemRegion<CC>(pt, regID,
		      selected ? m_selectedHiddenColor: m_hiddenColor, renderCache);
}

template <class CC>
void TileRenderer::RenderVisibleRegionOutlines(SPoint& pt, bool renderCache)
{
  int regID = renderCache?2:1;
  RenderMemRegion<CC>(pt, regID, 0xff202020, renderCache);
}

template <class CC>
void TileRenderer::RenderMemRegion(SPoint& pt, int regID,
				   Uint32 color, bool renderCache)
{
  // Extract short names for parameter types
  typedef typename CC::ATOM_TYPE T;
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

template <class CC>
void TileRenderer::RenderGrid(SPoint* pt, bool renderCache)
{
  // Extract short type names
  typedef typename CC::ATOM_TYPE T;
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

