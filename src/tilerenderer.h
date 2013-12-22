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
  
  u32 m_gridColor;
  u32 m_sharedColor;
  u32 m_visibleColor;
  u32 m_hiddenColor;

  Point<int> m_windowTL;
  SDL_Surface* m_dest;

  void RenderMemRegions(Point<int>* pt);

  void RenderMemRegion(Point<int>* pt, int regID,
		       u32 color);

  void RenderGrid(Point<int>* pt);

public:
  TileRenderer(SDL_Surface* dest);

  template <class T>
  void RenderTile(Tile<T>* t, Point<int>* loc)
  {
    Point<int> multPt(loc->GetX(), loc->GetY());

    multPt.Multiply(TILE_WIDTH * m_atomDrawSize);
    
    if(m_drawMemRegions)
    {
      RenderMemRegions(&multPt);
    }
    if(m_drawGrid)
    {
      RenderGrid(&multPt);
    }
  }

  void ToggleGrid();
  
  void ToggleMemDraw();
};


#endif /*TILERENDERER_H*/
