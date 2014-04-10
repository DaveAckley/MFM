#ifndef TILERENDERER_H      /* -*- C++ -*- */
#define TILERENDERER_H

#include "drawing.h"
#include "ElementTable.h"
#include "Tile.h"
#include "panel.h"
#include "Point.h"
#include "SDL/SDL.h"


namespace MFM {


class TileRenderer
{
private:
  bool m_drawGrid;
  enum DrawRegionType { NO, EDGE, FULL, MAX} m_drawMemRegions;
  bool m_drawDataHeat;
  u32 m_atomDrawSize;

  u32 m_gridColor;

  u32 m_cacheColor;
  u32 m_sharedColor;
  u32 m_visibleColor;
  u32 m_hiddenColor;
  u32 m_selectedHiddenColor;



  SPoint m_windowTL;

  Point<u32> m_dimensions;

  SDL_Surface* m_dest;

  template <class CC>
  void RenderMemRegions(SPoint& pt, bool renderCache, bool selected);

  template <class CC>
  void RenderVisibleRegionOutlines(SPoint& pt, bool renderCache, bool selected);

  template <class CC>
  void RenderMemRegion(SPoint& pt, int regID,
		       u32 color, bool renderCache);

  template <class CC>
  void RenderGrid(SPoint* pt, bool renderCache);

  void RenderAtomBG(SPoint& offset, Point<int>& atomloc,
		    u32 color);

  template <class CC>
  void RenderAtoms(SPoint& pt, Tile<CC>& tile, bool renderCache);

  template <class CC>
  u32 GetAtomColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom, u32 selector = 0);

  template <class CC>
  u32 GetDataHeatColor(Tile<CC>& tile, const typename CC::ATOM_TYPE& atom);

  template <class CC>
  void RenderEventWindow(SPoint& offset, Tile<CC>& tile, bool renderCache);

public:

  TileRenderer(SDL_Surface* dest);

  template <class CC>
  void RenderTile(Tile<CC>& t, SPoint& loc, bool renderWindow,
		  bool renderCache, bool selected);

  void SetDimensions(Point<u32> dimensions)
  {
    m_dimensions = dimensions;
  }

  SPoint& GetWindowTL()
  {
    return m_windowTL;
  }

  SDL_Surface* GetDestination()
  {
    return m_dest;
  }

  void IncreaseAtomSize();

  void DecreaseAtomSize();

  u32 GetAtomSize()
  {
    return m_atomDrawSize;
  }

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

