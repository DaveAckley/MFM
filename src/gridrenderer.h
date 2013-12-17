#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "grid.h"
#include "SDL/SDL.h"
#include "tilerenderer.h"
class GridRenderer
{
private:
  SDL_Surface* m_dest;
  TileRenderer* m_tileRenderer;

public:
  GridRenderer(SDL_Surface* dest);

  GridRenderer(SDL_Surface* dest,
	       TileRenderer* tr);

  ~GridRenderer();

  void RenderGrid(SDL_Surface* dest, Grid* grid);
  
};

#endif /*GRIDRENDERER_H*/
