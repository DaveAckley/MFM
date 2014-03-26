#include "gridrenderer.h"

namespace MFM {

  GridRenderer::GridRenderer(SDL_Surface* dest)
  {
    m_tileRenderer = new TileRenderer(dest);
    m_dest = dest;
    m_currentEWRenderMode = m_defaultRenderMode;
    m_renderTilesSeparated = m_renderTilesSeparatedDefault;
  }

  GridRenderer::GridRenderer(TileRenderer* tr)
  {
    m_tileRenderer = tr;
    m_dest = tr->GetDestination();
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

} /* namespace MFM */

