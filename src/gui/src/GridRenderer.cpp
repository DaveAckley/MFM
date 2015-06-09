#include "GridRenderer.h"

namespace MFM
{
  void GridRenderer::SaveDetails(ByteSink & sink) const
  {
    sink.Printf(",%d",m_renderTilesSeparated);
    sink.Printf(",%d",m_currentEWRenderMode);
    m_tileRenderer.SaveDetails(sink);
  }

  bool GridRenderer::LoadDetails(LineCountingByteSource & source)
  {
    u32 tmp_m_renderTilesSeparated;
    u32 tmp_m_currentEWRenderMode;
    if (2 != source.Scanf(",%d",&tmp_m_renderTilesSeparated)) return false;
    if (2 != source.Scanf(",%d",&tmp_m_currentEWRenderMode)) return false;
    if (!m_tileRenderer.LoadDetails(source)) return false;
    m_renderTilesSeparated = tmp_m_renderTilesSeparated;
    m_currentEWRenderMode = (EventWindowRenderMode) tmp_m_currentEWRenderMode;
    return true;
  }

  GridRenderer::GridRenderer() :
    m_cloneOrigin(-1, -1)
  {
    m_currentEWRenderMode = m_defaultRenderMode;
    m_renderTilesSeparated = m_renderTilesSeparatedDefault;
    m_selectedTile.Set(-1, -1);
    m_selectedAtom.Set(-1, -1);
    m_hoveredAtom.Set(-1, -1);
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
    return m_tileRenderer.GetDimensions();
  }

  void GridRenderer::SetDimensions(Point<u32> dimensions)
  {
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

  void GridRenderer::DecreaseAtomSize(SPoint around)
  {
    m_tileRenderer.DecreaseAtomSize(around);
  }

  void GridRenderer::ToggleGrid()
  {
    m_tileRenderer.ToggleGrid();
  }

  u32 GridRenderer::NextDrawBackgroundType()
  {
    return m_tileRenderer.NextDrawBackgroundType();
  }

  const SPoint & GridRenderer::GetDrawOrigin() const
  {
    return m_tileRenderer.GetWindowTL();
  }

  void GridRenderer::SetDrawOrigin(const SPoint & origin)
  {
    m_tileRenderer.SetWindowTL(origin);
  }

  void GridRenderer::SetCloneOrigin(const SPoint& cloneOrigin)
  {
    m_cloneOrigin.Set(cloneOrigin.GetX(), cloneOrigin.GetY());
  }

  void GridRenderer::Move(SPoint amount)
  {
    m_tileRenderer.Move(amount);
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

  void GridRenderer::DeselectAtom()
  {
    m_selectedAtom.Set(-1, -1);
  }

  SPoint GridRenderer::GetSelectedTile() const
  {
    return m_selectedTile;
  }

} /* namespace MFM */
