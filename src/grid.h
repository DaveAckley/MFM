#ifndef GRID_H
#define GRID_H

#include "itype.h"
#include "tile.h"

template <class T>
class Grid
{
private:

  u32 m_width, m_height;
  
  Tile<T>* m_tiles;

  Tile<T>* GetTile(int position);

public:

  Grid(int width, int height);

  ~Grid();
  
  void SetStateFunc(u32 (*stateFunc)(T* atom));

  u32 GetHeight();

  u32 GetWidth();

  void PlaceAtom(T* atom, Point<int>* location);

  T* GetAtom(Point<int>* location);

  void Expand(int extraW, int extraH);

  void Resize(int newWidth, int newHeight);

  void FillNeighbors(int center_x, int center_y,
		     Tile<T>** out);
  
  Tile<T>* GetTile(Point<int> location);
  
  Tile<T>* GetTile(int x, int y);
};

#include "grid.tcc"

#endif /*GRID_H*/
