/* -*- C++ -*- */

namespace MFM {

  template <class GC>
  void GridRenderer::RenderGrid(Grid<GC>& grid)
  {
    SPoint current;
    SPoint eventLoc;
    for(u32 x = 0; x < grid.GetWidth(); x++)
    {
      current.SetX(x);
      for(u32 y = 0; y < grid.GetHeight(); y++)
      {
	current.SetY(y);

	bool renderEW = true;

	switch(m_currentEWRenderMode)
	{
	case EVENTWINDOW_RENDER_OFF:
	  renderEW = false; break;
	case EVENTWINDOW_RENDER_CURRENT:
	  grid.FillLastEventTile(eventLoc);
	  renderEW =
	    current.GetX() == eventLoc.GetX() &&
	    current.GetY() == eventLoc.GetY();
	  break;
	case EVENTWINDOW_RENDER_ALL:
	  renderEW = true; break;
	  break;
	default: break;
	}

	m_tileRenderer->RenderTile(grid.GetTile(x, y),
				   current, renderEW, m_renderTilesSeparated,
				   current.GetX() == (s32)m_selectedTile.GetX() &&
				   current.GetY() == (s32)m_selectedTile.GetY());
      }
    }
  }

  template <class GC>
  void GridRenderer::SelectTile(Grid<GC>& grid, SPoint& clickPt)
  {
    SPoint& offset = m_tileRenderer->GetWindowTL();

    SPoint& cp = clickPt;

    /* Offset it by the corner */
    cp.SetX(cp.GetX() - offset.GetX());
    cp.SetY(cp.GetY() - offset.GetY());

    u32 tileSize = m_tileRenderer->GetAtomSize() * (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH + 1);
    if(!m_renderTilesSeparated)
    {
      tileSize -= m_tileRenderer->GetAtomSize() * 
	          (GC::CORE_CONFIG::PARAM_CONFIG::EVENT_WINDOW_RADIUS * 2 + 1);
    }

    m_selectedTile.Set(-1, -1);

    if(cp.GetX() > 0 && cp.GetY() > 0)
    {
      for(u32 x = 0; x < GC::GRID_WIDTH + 1; x++)
      {
	if(x * tileSize >= (u32)cp.GetX())
	{
	  m_selectedTile.SetX(x - 1);
	  break;
	}
      }
      for(u32 y = 0; y < GC::GRID_HEIGHT + 1; y++)
      {
	if(y * tileSize >= (u32)cp.GetY())
	{
	  m_selectedTile.SetY(y - 1);
	  break;
	}
      }
    }
  }

} /* namespace MFM */

