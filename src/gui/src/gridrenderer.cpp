#include "gridrenderer.h"

GridRenderer::GridRenderer(Panel* panel)
{
  m_tileRenderer = new TileRenderer(panel);
  m_panel = panel;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::GridRenderer(Panel* panel,
			   TileRenderer* tr)
{
  m_panel = panel;
  m_tileRenderer = tr;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::GridRenderer()
{
  m_tileRenderer = new TileRenderer(NULL);
  m_panel = NULL;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::~GridRenderer()
{
  delete m_tileRenderer;
}
