/*
template <class T>
void TileRenderer::RenderAtoms(Point<int>* pt,
			       Tile<T>* tile,
			       bool renderCache)
{
  Point<int> atomLoc;
  int ewr = EVENT_WINDOW_RADIUS;
  u32 (*stfu)(T*) = tile->GetStateFunc();
  u32 cacheOffset = renderCache ? ewr * m_atomDrawSize : 0;

  u32 astart = renderCache ? 0 : ewr;
  u32 aend   = renderCache ? TILE_WIDTH : TILE_WIDTH - ewr;
  for(int x = astart; x < aend; x++)
  {
    atomLoc.SetX(x);
    for(int y = astart; y < aend; y++)
    {
      atomLoc.SetY(y);
      u32 sval = stfu(tile->GetAtom(&atomLoc));
      u32 color;

      switch(sval)
      {
      case ELEMENT_DREG:
	color = 0xff505050;
	break;
      case ELEMENT_RES:
	color = 0xffffff00;
	break;
      default: continue;
      }
      Drawing::FillCircle(m_dest,
			  m_atomDrawSize *
			  atomLoc.GetX() +
			  pt->GetX() +
			  m_windowTL.GetX() +
			  cacheOffset,
			  m_atomDrawSize * 
			  atomLoc.GetY() +
			  pt->GetY() +
			  cacheOffset +
			  m_windowTL.GetY(),
			  m_atomDrawSize,
			  m_atomDrawSize,
			  (m_atomDrawSize / 2) - 2,
			  color);
    }
  }
}
*/

template <class T>
void TileRenderer::RenderAtoms(Point<int>* pt, Tile<T>* tile,
			       bool renderCache)
{
  u32 astart = renderCache ? 0 : EVENT_WINDOW_RADIUS;
  u32 aend   = renderCache ? TILE_WIDTH : TILE_WIDTH - EVENT_WINDOW_RADIUS;

  u32 cacheOffset = renderCache ? EVENT_WINDOW_RADIUS * m_atomDrawSize : 0;

  Point<int> atomLoc;

  for(int x = astart; x < aend; x++)
  {
    atomLoc.SetX(x);
    for(int y = astart; y < aend; y++)
    {
      atomLoc.SetY(y);

      T* atom = tile->GetAtom(&atomLoc);
      u32 color;

      switch(tile->GetStateFunc()(atom))
      {
      case ELEMENT_DREG:
	color = 0xff505050;
	break;
      case ELEMENT_RES:
	color = 0xffffff00;
	break;
      default: continue;
      }

      Drawing::FillCircle(m_dest,
			  pt->GetX() +
			  m_atomDrawSize * x +
			  m_windowTL.GetX() +
			  cacheOffset,
			  pt->GetY() +
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

template <class T>
void TileRenderer::RenderTile(Tile<T>* t, Point<int> loc, bool renderWindow,
			      bool renderCache)
{
  Point<int> multPt(loc.GetX(), loc.GetY());

  multPt.Multiply((TILE_WIDTH - 4) * 
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
      RenderMemRegions(multPt, renderCache);
    }

    if(renderWindow)
    {
      RenderEventWindow(multPt, *t, renderCache);
    }

    RenderAtoms(&multPt, t, renderCache);
    
    if(m_drawGrid)
    {
      RenderGrid(&multPt, renderCache);
    }
  }  
}

template <class T>
void TileRenderer::RenderEventWindow(Point<int>& offset,
				     Tile<T>& tile, bool renderCache)
{
  Point<int> winCenter;
  tile.FillLastExecutedAtom(winCenter);

  Point<int> atomLoc;
  Point<int> eventCenter;
  u32 cacheOffset = renderCache ? EVENT_WINDOW_RADIUS : 0;
  u32 drawColor = Drawing::WHITE;
  
  tile.FillLastExecutedAtom(eventCenter);
  u32 tableSize = ManhattanDir::GetTableSize(MANHATTAN_TABLE_EVENT);
  for(int i = 0; i < tableSize; i++)
  {
    ManhattanDir::FillFromBits(atomLoc, i, MANHATTAN_TABLE_EVENT);
    atomLoc.Add(eventCenter);
    atomLoc.Add(cacheOffset, cacheOffset);

    if(i == (tableSize >> 1))
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
