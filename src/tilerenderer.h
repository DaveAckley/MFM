#ifndef TILERENDERER_H
#define TILERENDERER_H

#include "tile.h"
#include "point.h"
#include "SDL/SDL.h"

class TileRenderer
{
 private:
  bool m_drawGrid;
  bool m_drawMemRegions;
  int m_atomDrawSize;
  Uint32 m_gridColor;
  Point<int> m_windowTL;
  SDL_Surface* m_dest;

  void RenderGrid(Point<int>* pt);

 public:
  TileRenderer(SDL_Surface* dest);

  void RenderTile(Tile* t, Point<int>* pt);

  void ToggleGrid();
};

#endif /*TILERENDERER_H*/
