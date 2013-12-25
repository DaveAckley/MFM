#ifndef TILERENDERER_H
#define TILERENDERER_H

#include "drawing.h"
#include "elementtable.hpp"
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

  template <class T>
  void RenderAtoms(Point<int>* pt, Tile<T>* tile);

public:

  TileRenderer(SDL_Surface* dest);

  template <class T>
  void RenderTile(Tile<T>* t, Point<int>* loc)
  {
    Point<int> multPt(loc->GetX(), loc->GetY());

    multPt.Multiply((TILE_WIDTH - 4) * 
		    m_atomDrawSize);

    if(multPt.GetX() >= 0 && multPt.GetY() >= 0 &&
       multPt.GetX() < m_dest->w &&
       multPt.GetY() < m_dest->h)
    {
      if(m_drawMemRegions)
      {
	RenderMemRegions(&multPt);
      }

      RenderAtoms(&multPt, t);

      if(m_drawGrid)
      {
	RenderGrid(&multPt);
      }
    }
  }



  void IncreaseAtomSize();

  void DecreaseAtomSize();

  void ToggleGrid();
  
  void ToggleMemDraw();
};

template <class T>
void TileRenderer::RenderAtoms(Point<int>* pt,
			       Tile<T>* tile)
{
  Point<int> atomLoc;
  int ewr = EVENT_WINDOW_RADIUS;
  u32 (*stfu)(T*) = tile->GetStateFunc();
  for(int x = ewr; x < TILE_WIDTH - ewr; x++)
  {
    atomLoc.SetX(x - ewr);
    for(int y = ewr; y < TILE_WIDTH - ewr; y++)
    {
      atomLoc.SetY(y - ewr);
      u32 sval = stfu(tile->GetAtom(&atomLoc));
      
      u32 color;

      switch(sval)
      {
      case ELEMENT_DREG:
	color = 0xff505050;
	break;
      default: continue;
      }
      Drawing::FillCircle(m_dest,
			  m_atomDrawSize *
			  atomLoc.GetX() +
			  pt->GetX(),
			  m_atomDrawSize * 
			  atomLoc.GetY() +
			  pt->GetY(),
			  m_atomDrawSize,
			  m_atomDrawSize,
			  (m_atomDrawSize / 2) - 2,
			  0xff00ff00);
    }
  }
}


#endif /*TILERENDERER_H*/
