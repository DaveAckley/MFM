#ifndef GRID_H      /* -*- C++ -*- */
#define GRID_H

#include "itype.h"
#include "tile.h"
#include "elementtable.h"

template <class T,u32 R>
class Grid
{
private:
  u32 m_width, m_height;

  Point<int> m_lastEventTile;
  
  Tile<T,R>* m_tiles;

  ElementTable<T,R>* m_elementTable;

  Tile<T,R>* GetTile(int position);

  inline u32 GetPosition(u32 x, u32 y) 
  { return x + y * m_width; }
  
  inline Tile<T,R> & GetTile(const Point<u32>& pt)
  {return m_tiles[GetPosition(pt.GetX(), pt.GetY())];}

  inline Tile<T,R> & GetTile(u32 x, u32 y) 
  { return m_tiles[GetPosition(x, y)]; }

  inline const Tile<T,R> & GetTile(u32 x, u32 y) const 
  { return m_tiles[GetPosition(x, y)]; }

public:

  friend class GridRenderer;

  Grid(int width, int height, ElementTable<T,R>* elementTable);

  ~Grid();
  
  void SetStateFunc(u32 (*stateFunc)(T* atom));

  u32 GetHeight();

  u32 GetWidth();

  void PlaceAtom(T& atom, Point<int>& location);

  T* GetAtom(Point<int>& location);

  void Expand(int extraW, int extraH);

  void Resize(int newWidth, int newHeight);

  void FillLastEventTile(Point<int>& out);

  void TriggerEvent();
  
  Tile<T,R>& GetTile(Point<int> location);
  
  Tile<T,R>& GetTile(int x, int y);
};

#include "grid.tcc"

#endif /*GRID_H*/
