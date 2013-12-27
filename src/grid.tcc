#include "eucliddir.h"
#include "grid.h"

template <class T>
Grid<T>::Grid(int width, int height, ElementTable<T>* elementTable)
{
  m_width = width;
  m_height = height;

  m_tiles = new Tile<T>[m_width * m_height];

  m_elementTable = elementTable;
}

template <class T>
void Grid<T>::SetStateFunc(u32 (*stateFunc)(T* atom))
{
  for(int i = 0; i < m_width * m_height; i++)
  {
    m_tiles[i].SetStateFunc(stateFunc);
  }
}

template <class T>
Grid<T>::~Grid()
{
  delete[] m_tiles;
}

template <class T>
u32 Grid<T>::GetHeight()
{
  return m_height;
}

template <class T>
u32 Grid<T>::GetWidth()
{
  return m_width;
}

template <class T>
void Grid<T>::PlaceAtom(T* atom,
			Point<int>* loc)
{
  int x = loc->GetX() / TILE_WIDTH;
  int y = loc->GetY() / TILE_WIDTH;
  Tile<T>& tile = m_tiles[x + y * m_width];

  Point<int> local(loc->GetX() % TILE_WIDTH,
		   loc->GetY() % TILE_WIDTH);

  m_tiles[x + y * m_width].PlaceAtom(atom, &local);
}

template <class T>
T* Grid<T>::GetAtom(Point<int>* loc)
{
  int x = loc->GetX() / TILE_WIDTH;
  int y = loc->GetY() / TILE_WIDTH;
  Tile<T>& tile = m_tiles[x + y * m_width];

  Point<int> local(loc->GetX() % TILE_WIDTH,
		   loc->GetY() % TILE_WIDTH);

  return m_tiles[x + y * m_width].GetAtom(&local);
}

template <class T>
void Grid<T>::Expand(int extraW, int extraH)
{
  Resize(m_width + extraW, m_height + extraH);
}

template <class T>
void Grid<T>::Resize(int newWidth, int newHeight)
{
  delete m_tiles;

  m_width = newWidth;
  m_height = newHeight;

  m_tiles = new Tile<T>[m_width * m_height];
}

template <class T>
void Grid<T>::TriggerEvent()
{
  Point<int> windowTile(true, m_width, m_height);

  m_tiles[windowTile.GetX() + 
	  windowTile.GetY() * m_width].Execute(*m_elementTable);

  m_lastEventTile.Set(windowTile.GetX(), windowTile.GetY());
}

template <class T>
void Grid<T>::FillNeighbors(int center_x, int center_y,
			    Tile<T>** out)
{  
  for(int i = 0; i < 8; i++)
  {
    
  }
}

template <class T>
Tile<T>* Grid<T>::GetTile(int x, int y)
{
  return &m_tiles[y * m_width + x];
}
