#ifndef TILERENDERER_H      /* -*- C++ -*- */
#define TILERENDERER_H

#include "drawing.h"
#include "elementtable.h"
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
  u32 m_cacheColor;

  Point<int> m_windowTL;
  SDL_Surface* m_dest;

  void RenderMemRegions(Point<int>& pt, bool renderCache);

  void RenderMemRegion(Point<int>& pt, int regID,
		       u32 color, bool renderCache);

  void RenderGrid(Point<int>* pt, bool renderCache);

  void RenderAtomBG(Point<int>& offset, Point<int>& atomloc,
		    u32 color);

  template <class T>
  void RenderAtoms(Point<int>& pt, Tile<T>& tile, bool renderCache);

  template <class T>
  void RenderEventWindow(Point<int>& offset, Tile<T>& tile, bool renderCache);

public:

  TileRenderer(SDL_Surface* dest);

  template <class T>
  void RenderTile(Tile<T>& t, Point<int>& loc, bool renderWindow, 
		  bool renderCache);

  void IncreaseAtomSize();

  void DecreaseAtomSize();

  void ToggleGrid();
  
  void ToggleMemDraw();

  void MoveUp(u8 amount);

  void MoveDown(u8 amount);

  void MoveLeft(u8 amount);

  void MoveRight(u8 amount);
};

#include "tilerenderer.tcc"

#endif /*TILERENDERER_H*/
