template <class T, u32 R>
u32 TileRenderer::GetAtomColor(Tile<T,R>& tile, T& atom)
{
  switch(tile.GetStateFunc()(&atom))
  {
  case ELEMENT_DREG:
    return 0xff505050;
  case ELEMENT_RES:
    return 0xffffff00;
  case ELEMENT_SORTER:
    return 0xffff0000;
  case ELEMENT_EMITTER:
    return 0xff808080;
  case ELEMENT_DATA:
    return 0xff0000ff;
  case ELEMENT_CONSUMER:
    return 0xff101010;
  }
  return 0;
}

template <class T, u32 R>
u32 TileRenderer::GetDataHeatColor(Tile<T,R>& tile, T& atom)
{
  if(tile.GetStateFunc()(&atom) == ELEMENT_DATA)
  {
    u8 tcl = atom.ReadLowerBits();
    return 0xff000000 | (tcl << 17);
  }
  return 0;
}


template <class T,u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderAtoms(Point<int>& pt, Tile<T,EVENT_WINDOW_RADIUS>& tile,
			       bool renderCache)
{
  u32 astart = renderCache ? 0 : EVENT_WINDOW_RADIUS;
  u32 aend   = renderCache ? TILE_WIDTH : TILE_WIDTH - EVENT_WINDOW_RADIUS;

  u32 cacheOffset = renderCache ? 0 : -EVENT_WINDOW_RADIUS * m_atomDrawSize;

  Point<int> atomLoc;

  for(u32 x = astart; x < aend; x++)
  {
    atomLoc.SetX(x);
    for(u32 y = astart; y < aend; y++)
    {
      atomLoc.SetY(y);

      T* atom = tile.GetAtom(&atomLoc);
      u32 color;
      if(m_drawDataHeat)
      {
	color = GetDataHeatColor(tile, *atom);
      }
      else
      {
	color = GetAtomColor(tile, *atom);
      }

      if(color)
      {
	Drawing::FillCircle(m_dest,
			    pt.GetX() +
			    m_atomDrawSize * x +
			    m_windowTL.GetX() +
			    cacheOffset,
			    pt.GetY() +
			    m_atomDrawSize * y +
			    m_windowTL.GetY() +
			    cacheOffset,
			    m_atomDrawSize,
			    m_atomDrawSize,
			    (m_atomDrawSize / 2) - 2,
			    color);
      }
    }
  }
}

template <class T,u32 R>
void TileRenderer::RenderTile(Tile<T,R>& t, Point<int>& loc, bool renderWindow,
			      bool renderCache)
{
  Point<int> multPt(loc);

  multPt.Multiply((TILE_WIDTH - R * 3) * 
		  m_atomDrawSize);

  Point<int> realPt(multPt.GetX(), multPt.GetY());


  u32 tileHeight = TILE_WIDTH * m_atomDrawSize;

  realPt.Add(m_windowTL);

  if(realPt.GetX() + tileHeight >= 0 &&
     realPt.GetY() + tileHeight >= 0 &&
     realPt.GetX() < m_dest->w &&
     realPt.GetY() < m_dest->h)
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
void TileRenderer::RenderEventWindow(Point<int>& offset,
				     Tile<T,R>& tile, bool renderCache)
{
  Point<int> winCenter;
  tile.FillLastExecutedAtom(winCenter);

  Point<int> atomLoc;
  Point<int> eventCenter;
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
void TileRenderer::RenderMemRegions(Point<int>& pt, bool renderCache)
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
void TileRenderer::RenderMemRegion(Point<int>& pt, int regID,
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

  Drawing::FillRect(m_dest,
		    pt.GetX() + (ewrSize * regID) +
		    m_windowTL.GetX(),
		    pt.GetY() + (ewrSize * regID) +
		    m_windowTL.GetY(),
		    tileSize - (ewrSize * regID * 2),
		    tileSize - (ewrSize * regID * 2),
		    color);
}

template <u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderGrid(Point<int>* pt, bool renderCache)
{

  int lineLen, linesToDraw;

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

  for(int x = 0; x < linesToDraw; x++)
  {
    Drawing::DrawVLine(m_dest,
		       pt->GetX() + x * m_atomDrawSize +
		       m_windowTL.GetX(),
		       pt->GetY() + m_windowTL.GetY(),
		       pt->GetY() + lineLen +
		       m_windowTL.GetY(),
		       m_gridColor);
  }

  for(int y = 0; y < linesToDraw; y++)
  {
    Drawing::DrawHLine(m_dest,
		       pt->GetY() + y * m_atomDrawSize +
		       m_windowTL.GetY(),
		       pt->GetX() + m_windowTL.GetX(),
		       pt->GetX() + lineLen +
		       m_windowTL.GetX(),
		       m_gridColor);
  }
}
