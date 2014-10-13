/* -*- C++ -*- */

namespace MFM
{

  template <class GC>
  void GridRenderer::RenderGrid(Drawing & drawing, Grid<GC>& grid, u32 brushSize)
  {
    SPoint current;
    SPoint eventLoc;
    const u32 tileSize = m_tileRenderer.GetAtomSize() *
      (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH -
       2 * GC::CORE_CONFIG::PARAM_CONFIG::EVENT_WINDOW_RADIUS);
    const u32 atomSize = m_tileRenderer.GetAtomSize();
    SPoint atomTile(-1, -1);
    SPoint cloneTile(-1, -1);

    if(m_selectedAtom.GetX() >= 0 && m_selectedAtom.GetY() >= 0)
    {
      atomTile.Set(m_selectedAtom.GetX() / (tileSize / atomSize),
                   m_selectedAtom.GetY() / (tileSize / atomSize));
    }

    if(m_cloneOrigin.GetX() >= 0 && m_cloneOrigin.GetY() >= 0)
    {
      cloneTile.Set(m_cloneOrigin.GetX() / (tileSize / atomSize),
                    m_cloneOrigin.GetY() / (tileSize / atomSize));
    }

    for(u32 x = 0; x < grid.GetWidth(); x++)
    {
      current.SetX(x);
      for(u32 y = 0; y < grid.GetHeight(); y++)
      {
        current.SetY(y);

        SPoint selectedAtom;
        SPoint cloneAtom;
        SPoint* selectedAtomPtr = NULL;
        SPoint* cloneAtomPtr = NULL;

        if(atomTile.Equals(current))
        {
          selectedAtom.SetX(m_selectedAtom.GetX() - atomTile.GetX() * (tileSize / atomSize));
          selectedAtom.SetY(m_selectedAtom.GetY() - atomTile.GetY() * (tileSize / atomSize));
          selectedAtomPtr = &selectedAtom;
        }

        if(cloneTile.Equals(current))
        {
          cloneAtom.SetX(m_cloneOrigin.GetX() - cloneTile.GetX() * (tileSize / atomSize));
          cloneAtom.SetY(m_cloneOrigin.GetY() - cloneTile.GetY() * (tileSize / atomSize));
          cloneAtomPtr = &cloneAtom;
        }

        m_tileRenderer.RenderTile(drawing,
                                  grid.GetTile(x, y),
                                  current, false, m_renderTilesSeparated,
                                  current.GetX() == (s32)m_selectedTile.GetX() &&
                                  current.GetY() == (s32)m_selectedTile.GetY(),
                                  selectedAtomPtr,
                                  cloneAtomPtr);
      }
    }


    if(brushSize > 0)
    {
      const s32 haX = (s32)m_hoveredAtom.GetX();
      const s32 haY = (s32)m_hoveredAtom.GetY();
      drawing.SetForeground(Drawing::YELLOW);

      brushSize--;
      SPoint wtl = m_tileRenderer.GetWindowTL();
      const s32 brushSqr = brushSize * brushSize;
      s32 xdSqr;
      for(s32 x = haX - brushSize; x <= haX; x++)
      {
        xdSqr = (x - haX) * (x - haX);
        for(s32 y = haY - brushSize; y <= haY; y++)
        {
          if(x >= 0 && y >= 0)
          {
            if(((y - haY) * (y - haY)) + xdSqr <= brushSqr)
            {
              drawing.FillRect(wtl.GetX() + (x + 0.5) * atomSize,
                               wtl.GetY() + (y + 0.5) * atomSize, 2, 2);

              drawing.FillRect(wtl.GetX() + (x + 2 * (haX - x) + 0.5) * atomSize,
                               wtl.GetY() + (y + 0.5) * atomSize, 2, 2);

              drawing.FillRect(wtl.GetX() + (x + 0.5) * atomSize,
                               wtl.GetY() + (y + 2 * (haY - y) + 0.5) * atomSize, 2, 2);

              drawing.FillRect(wtl.GetX() + (x + 2 * (haX - x) + 0.5) * atomSize,
                               wtl.GetY() + (y + 2 * (haY - y) + 0.5) * atomSize, 2, 2);
            }
          }
        }
      }
    }
  }

  template <class GC>
  void GridRenderer::SelectAtom(Grid<GC>& grid, SPoint clickPt)
  {
    if(!m_renderTilesSeparated) /* Not going to perform cache mapping */
    {
      const SPoint& offset = m_tileRenderer.GetWindowTL();

      SPoint cp = clickPt;

      cp.SetX(cp.GetX() - offset.GetX());
      cp.SetY(cp.GetY() - offset.GetY());

      u32 atomSize = m_tileRenderer.GetAtomSize();

      u32 tileSize = atomSize *
        (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH -
         2 * GC::CORE_CONFIG::PARAM_CONFIG::EVENT_WINDOW_RADIUS);

      m_selectedAtom.Set(-1, -1);

      SPoint oldSelectedTile = m_selectedTile;
      SelectTile(grid, clickPt);

      m_selectedAtom.SetX(((cp.GetX() % tileSize) +
                           m_selectedTile.GetX() * tileSize) / atomSize);
      m_selectedAtom.SetY(((cp.GetY() % tileSize) +
                           m_selectedTile.GetY() * tileSize) / atomSize);

      m_selectedTile = oldSelectedTile;
    }
  }

  template <class GC>
  void GridRenderer::SetHoveredAtom(Grid<GC>& grid, SPoint clickPt)
  {
    SPoint selectedAtom(m_selectedAtom.GetX(), m_selectedAtom.GetY());

    SelectAtom(grid, clickPt);

    m_hoveredAtom.Set(m_selectedAtom.GetX(), m_selectedAtom.GetY());
    m_selectedAtom.Set(selectedAtom.GetX(), selectedAtom.GetY());
  }

  template <class GC>
  void GridRenderer::SelectTile(Grid<GC>& grid, SPoint clickPt)
  {
    const SPoint& offset = m_tileRenderer.GetWindowTL();

    SPoint& cp = clickPt;

    /* Offset it by the corner */
    cp.SetX(cp.GetX() - offset.GetX());
    cp.SetY(cp.GetY() - offset.GetY());

    u32 tileSize = m_tileRenderer.GetAtomSize() *
      (GC::CORE_CONFIG::PARAM_CONFIG::TILE_WIDTH + 1);

    if(!m_renderTilesSeparated)
    {
      tileSize -= m_tileRenderer.GetAtomSize() *
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
