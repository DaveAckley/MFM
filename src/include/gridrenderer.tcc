template <class T,u32 R>
void GridRenderer::RenderGrid(Grid<T,R>& grid)
{
  if(m_renderTilesSeparated)
  {
    RenderGridSeparated(grid);
  }
  else
  {
    RenderGridClose(grid);
  }
}

template <class T,u32 R>
void GridRenderer::RenderGridSeparated(Grid<T,R>& grid)
{
  Point<int> current;
  Point<int> eventLoc;
  for(u32 x = 0; x < grid.GetWidth(); x++)
  {
    current.SetX(x * 2);
    for(u32 y = 0; y < grid.GetHeight(); y++)
    {
      current.SetY(y * 2);
      
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
				 current, renderEW, true);
    }
  }  
}

template <class T,u32 R>
void GridRenderer::RenderGridClose(Grid<T,R>& grid)
{
  Point<int> current;
  Point<int> eventLoc;
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
				 current, renderEW, false);
    }
  }
}
