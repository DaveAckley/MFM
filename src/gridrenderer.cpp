#include "gridrenderer.h"

GridRenderer::GridRenderer(SDL_Surface* dest)
{
  m_tileRenderer = new TileRenderer(dest);
  m_dest = dest;
}

GridRenderer::GridRenderer(SDL_Surface* dest,
			   TileRenderer* tr)
{
  m_dest = dest;
  m_tileRenderer = tr;
}

GridRenderer::~GridRenderer()
{
  delete m_tileRenderer;
}

void RenderGrid(SDL_Surface* dest, Grid* grid)
{
  
}
