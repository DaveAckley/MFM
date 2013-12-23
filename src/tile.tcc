#include "elementtable.hpp"

template <class T>
T* Tile<T>::GetAtom(Point<int>* pt)
{
  return &m_atoms[pt->GetX() + 
		  pt->GetY() * TILE_WIDTH];
}

template <class T>
T* Tile<T>::GetAtom(int x, int y)
{
  return &m_atoms[x + y * TILE_WIDTH];
}

template <class T>
T* Tile<T>::GetAtom(int i)
{
  return &m_atoms[i];
}

template <class T>
EventWindow<T>* Tile<T>::CreateRandomWindow()
{
  Point<int> pt(true);

  /* Make sure not to be created in the cache */
  pt.Set(2 + pt.GetX() % (TILE_WIDTH - 4),
	 2 + pt.GetY() % (TILE_WIDTH - 4));

  
}

template <class T>
void Tile<T>::PlaceAtom(T* atom, Point<int>* pt)
{
  T dest = 
  m_atoms[pt->GetX() + 
	  pt->GetY() * TILE_WIDTH];

  dest = *atom;

  assert(dest.GetState() == atom->GetState());

  m_atoms[pt->GetX() +
	  pt->GetY() * TILE_WIDTH] = dest;

  
}

template <class T>
void Tile<T>::Execute()
{
  EventWindow<T>* window = CreateRandomWindow();
  
}

template <class T>
void Tile<T>::ResetAtom(Point<int>* point, ElementType type)
{

}
