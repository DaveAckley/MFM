#include "GridRenderer.h"

namespace MFM
{
  void GridRenderer::GridRendererSaveDetails(ByteSink & sink) const
  {
    sink.Printf(" PP(grrts=%d)\n",m_renderTilesSeparated);
    sink.Printf(" PP(grcrm=%d)\n",m_currentEWRenderMode);
    m_tileRenderer.TileRendererSaveDetails(sink);
  }

  bool GridRenderer::GridRendererLoadDetails(const char * key, LineCountingByteSource & source)
  {
    if (!strcmp("grrts",key)) return 1 == source.Scanf("%?d", sizeof m_renderTilesSeparated, &m_renderTilesSeparated);
    if (!strcmp("grcrm",key)) return 1 == source.Scanf("%?d", sizeof m_currentEWRenderMode, &m_currentEWRenderMode);
    return m_tileRenderer.TileRendererLoadDetails(key, source);
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

#if 0 // XXX KILLGREX
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
#endif

  void GridRenderer::ToggleTileSeparation()
  {
    m_renderTilesSeparated = !m_renderTilesSeparated;
  }

#if 0 // XXX KILLGREX
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
#endif

  void GridRenderer::SetCloneOrigin(const SPoint& cloneOrigin)
  {
    m_cloneOrigin.Set(cloneOrigin.GetX(), cloneOrigin.GetY());
  }

#if 0 // XXX KILLGREX
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
#endif

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
