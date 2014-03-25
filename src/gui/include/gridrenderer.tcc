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
                                       current, renderEW, m_renderTilesSeparated);
          }
      }  
  }

} /* namespace MFM */

