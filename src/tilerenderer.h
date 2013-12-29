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

  void RenderMemRegions(Point<int>& pt);

  void RenderMemRegion(Point<int>& pt, int regID,
		       u32 color);

  void RenderGrid(Point<int>* pt);

  void RenderAtomBG(Point<int>& offset, Point<int>& atomloc,
		    u32 color);

  template <class T>
  void RenderAtoms(Point<int>* pt, Tile<T>* tile);

  template <class T>
  void RenderEventWindow(Point<int>& offset, Tile<T>& tile);

public:

  TileRenderer(SDL_Surface* dest);

  template <class T>
  void RenderTile(Tile<T>* t, Point<int> loc, bool renderWindow)
  {
    Point<int> multPt(loc.GetX(), loc.GetY());

    multPt.Multiply((TILE_WIDTH - 4) * 
		    m_atomDrawSize);

    Point<int> realPt(multPt.GetX(), multPt.GetY());


    u32 tileHeight = TILE_WIDTH * m_atomDrawSize;

    realPt.Add(m_windowTL);

    if(realPt.GetX() + tileHeight >= 0 &&
       realPt.GetY() + tileHeight >= 0 &&
       realPt.GetX() < m_dest->w &&
       realPt.GetY() < m_dest->h)
    {
      if(m_drawMemRegions)
      {
	RenderMemRegions(multPt);
      }

      RenderAtoms(&multPt, t);

      if(renderWindow)
      {
	RenderEventWindow(multPt, *t);
      }

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

  void MoveUp(u8 amount);

  void MoveDown(u8 amount);

  void MoveLeft(u8 amount);

  void MoveRight(u8 amount);
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
			  pt->GetX() +
			  m_windowTL.GetX(),
			  m_atomDrawSize * 
			  atomLoc.GetY() +
			  pt->GetY() +
			  m_windowTL.GetY(),
			  m_atomDrawSize,
			  m_atomDrawSize,
			  (m_atomDrawSize / 2) - 2,
			  color);
    }
  }
}

template <class T>
void TileRenderer::RenderEventWindow(Point<int>& offset,
				     Tile<T>& tile)
{
  Point<int> winCenter;
  tile.FillLastExecutedAtom(winCenter);

  Point<int> atomLoc;
  Point<int> eventCenter;
  u32 drawColor = Drawing::WHITE;
  
  tile.FillLastExecutedAtom(eventCenter);
  u32 tableSize = ManhattanDir::GetTableSize(MANHATTAN_TABLE_EVENT);
  for(int i = 0; i < tableSize; i++)
  {
    ManhattanDir::FillFromBits(atomLoc, i, MANHATTAN_TABLE_EVENT);
    atomLoc.Add(eventCenter);

    if(i == (tableSize >> 1))
    {
      drawColor = Drawing::GREEN;
    }
    else
    {
      drawColor = Drawing::WHITE;
    }

    RenderAtomBG(offset, atomLoc, drawColor);
  }
}


#endif /*TILERENDERER_H*/
