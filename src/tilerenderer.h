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
  Uint32 m_sharedColor;
  Uint32 m_visibleColor;
  Uint32 m_hiddenColor;

  Point<int> m_windowTL;
  SDL_Surface* m_dest;

  void RenderMemRegions(Point<int>* pt);

  void RenderMemRegion(Point<int>* pt, int regID,
		       Uint32 color);

  void RenderGrid(Point<int>* pt);

 public:
  TileRenderer(SDL_Surface* dest);

  void RenderTile(Tile* t, Point<int>* pt);

  void ToggleGrid();

  void ToggleMemDraw();
};

#endif /*TILERENDERER_H*/
