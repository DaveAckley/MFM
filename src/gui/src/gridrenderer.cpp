#include "gridrenderer.h"

GridRenderer::GridRenderer(SDL_Surface* dest)
{
  m_tileRenderer = new TileRenderer(dest);
  m_dest = dest;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::GridRenderer(SDL_Surface* dest,
			   TileRenderer* tr)
{
  m_dest = dest;
  m_tileRenderer = tr;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::GridRenderer()
{
  m_tileRenderer = new TileRenderer(NULL);
  m_dest = NULL;
  m_currentEWRenderMode = m_defaultRenderMode;
  m_renderTilesSeparated = m_renderTilesSeparatedDefault;
}

GridRenderer::~GridRenderer()
{
  delete m_tileRenderer;
}
