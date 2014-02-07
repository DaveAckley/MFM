#ifndef TILERENDERER_H      /* -*- C++ -*- */
#define TILERENDERER_H

#include "drawing.h"
#include "elementtable.h"
#include "tile.h"
#include "panel.h"
#include "point.h"
#include "SDL/SDL.h"


namespace MFM {


class TileRenderer
{
private:
  bool m_drawGrid;
  bool m_drawMemRegions;
  bool m_drawDataHeat;
  int m_atomDrawSize;
  
  u32 m_gridColor;
  u32 m_sharedColor;
  u32 m_visibleColor;
  u32 m_hiddenColor;
  u32 m_cacheColor;

  SPoint m_windowTL;

  Point<u32> m_dimensions;

  SDL_Surface* m_dest;

  template <u32 R>
  void RenderMemRegions(SPoint& pt, bool renderCache);

  template <u32 R>
  void RenderMemRegion(SPoint& pt, int regID,
		       u32 color, bool renderCache);

  template <u32 R>
  void RenderGrid(SPoint* pt, bool renderCache);

  void RenderAtomBG(SPoint& offset, Point<int>& atomloc,
		    u32 color);

  template <class T,u32 R>
  void RenderAtoms(SPoint& pt, Tile<T,R>& tile, bool renderCache);

  template <class T, u32 R>
  u32 GetAtomColor(Tile<T,R>& tile, T& atom);

  template <class T, u32 R>
  u32 GetDataHeatColor(Tile<T,R>& tile, T& atom);

  template <class T,u32 R>
  void RenderEventWindow(SPoint& offset, Tile<T,R>& tile, bool renderCache);

public:

  TileRenderer(SDL_Surface* dest);

  template <class T,u32 R>
  void RenderTile(Tile<T,R>& t, SPoint& loc, bool renderWindow, 
		  bool renderCache);

  void SetDimensions(Point<u32> dimensions)
  {
    m_dimensions = dimensions;
  }

  SDL_Surface* GetDestination()
  {
    return m_dest;
  }

  void IncreaseAtomSize();

  void DecreaseAtomSize();

  void ToggleGrid();
  
  void ToggleMemDraw();

  void ToggleDataHeat();

  void MoveUp(u8 amount);

  void MoveDown(u8 amount);

  void MoveLeft(u8 amount);

  void MoveRight(u8 amount);
};
} /* namespace MFM */

#include "tilerenderer.tcc"

#endif /*TILERENDERER_H*/

