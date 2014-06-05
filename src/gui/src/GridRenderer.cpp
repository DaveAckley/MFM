#include "GridRenderer.h"

namespace MFM {

  /*
  GridRenderer::GridRenderer(SDL_Surface* dest)
  {
    m_tileRenderer = new TileRenderer(dest);
    m_dest = dest;
    m_currentEWRenderMode = m_defaultRenderMode;
    m_renderTilesSeparated = m_renderTilesSeparatedDefault;
    m_selectedTile.Set(-1, -1);
  }
  */

  /*
  GridRenderer::GridRenderer(TileRenderer* tr)
  {
    m_tileRenderer = tr;
    //    m_dest = tr->GetDestination();
    m_currentEWRenderMode = m_defaultRenderMode;
    m_renderTilesSeparated = m_renderTilesSeparatedDefault;
    m_selectedTile.Set(-1, -1);
  }
  */

  GridRenderer::GridRenderer()
  {
    m_currentEWRenderMode = m_defaultRenderMode;
    m_renderTilesSeparated = m_renderTilesSeparatedDefault;
    m_selectedTile.Set(-1, -1);
  }

  GridRenderer::~GridRenderer()
  {
  }

  void GridRenderer::SetEventWindowRenderMode(EventWindowRenderMode mode)
  {
    m_currentEWRenderMode = mode;
  }

  UPoint GridRenderer::GetDimensions() const
  {
    return m_dimensions;
  }

  void GridRenderer::SetDimensions(Point<u32> dimensions)
  {
    m_dimensions = dimensions;
    m_tileRenderer.SetDimensions(dimensions);
  }

  void GridRenderer::IncreaseAtomSize(SPoint around)
  {
    m_tileRenderer.IncreaseAtomSize(around);
  }

  void GridRenderer::ToggleTileSeparation()
  {
    m_renderTilesSeparated = !m_renderTilesSeparated;
  }

  void GridRenderer::ToggleDataHeatmap()
  {
    m_tileRenderer.ToggleDataHeat();
  }

  void GridRenderer::DecreaseAtomSize(SPoint around)
  {
    m_tileRenderer.DecreaseAtomSize(around);
  }

  void GridRenderer::ToggleGrid()
  {
    m_tileRenderer.ToggleGrid();
  }

  void GridRenderer::ToggleMemDraw()
  {
    m_tileRenderer.ToggleMemDraw();
  }

  const SPoint & GridRenderer::GetDrawOrigin() const
  {
    return m_tileRenderer.GetWindowTL();
  }

  void GridRenderer::SetDrawOrigin(const SPoint & origin)
  {
    m_tileRenderer.SetWindowTL(origin);
  }

  void GridRenderer::MoveUp(u8 amount)
  {
    m_tileRenderer.MoveUp(amount);
  }

  void GridRenderer::MoveDown(u8 amount)
  {
    m_tileRenderer.MoveDown(amount);
  }

  void GridRenderer::MoveLeft(u8 amount)
  {
    m_tileRenderer.MoveLeft(amount);
  }

  void GridRenderer::MoveRight(u8 amount)
  {
    m_tileRenderer.MoveRight(amount);
  }

  void GridRenderer::DeselectTile()
  {
    m_selectedTile.Set(-1, -1);
  }

  SPoint GridRenderer::GetSelectedTile() const
  {
    return m_selectedTile;
  }

} /* namespace MFM */

