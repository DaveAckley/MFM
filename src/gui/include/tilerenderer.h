#ifndef TILERENDERER_H      /* -*- C++ -*- */
#define TILERENDERER_H

#include "drawing.h"
#include "elementtable.h"
#include "tile.h"
#include "panel.h"
#include "point.h"
#include "SDL/SDL.h"



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

  Point<int> m_windowTL;

  Panel* m_panel;

  template <u32 R>
  void RenderMemRegions(Point<int>& pt, bool renderCache);

  template <u32 R>
  void RenderMemRegion(Point<int>& pt, int regID,
		       u32 color, bool renderCache);

  template <u32 R>
  void RenderGrid(Point<int>* pt, bool renderCache);

  void RenderAtomBG(Point<int>& offset, Point<int>& atomloc,
		    u32 color);

  template <class T,u32 R>
  void RenderAtoms(Point<int>& pt, Tile<T,R>& tile, bool renderCache);

  template <class T, u32 R>
  u32 GetAtomColor(Tile<T,R>& tile, T& atom);

  template <class T, u32 R>
  u32 GetDataHeatColor(Tile<T,R>& tile, T& atom);

  template <class T,u32 R>
  void RenderEventWindow(Point<int>& offset, Tile<T,R>& tile, bool renderCache);

public:

  TileRenderer(Panel* panel);

  template <class T,u32 R>
  void RenderTile(Tile<T,R>& t, Point<int>& loc, bool renderWindow, 
		  bool renderCache);

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

#include "tilerenderer.tcc"

#endif /*TILERENDERER_H*/
