#include "manhattandir.h"

template <class T,u32 R>
T* Tile<T,R>::GetAtom(Point<int>* pt)
{
  return &m_atoms[pt->GetX() + 
		  pt->GetY() * TILE_WIDTH];
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(int x, int y)
{
  return &m_atoms[x + y * TILE_WIDTH];
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(int i)
{
  return &m_atoms[i];
}

template <class T,u32 R>
void Tile<T,R>::FillLastExecutedAtom(Point<int>& out)
{
  out.Set(m_lastExecutedAtom.GetX(),
	  m_lastExecutedAtom.GetY());
}

template <class T,u32 R>
void Tile<T,R>::CreateRandomWindow()
{
  /* Make sure not to be created in the cache */
  int maxval = TILE_WIDTH - (EVENT_WINDOW_RADIUS << 1);
  Point<int> pt(true, maxval, maxval);
  pt.Add(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);

  m_executingWindow = EventWindow<T,R>(pt, m_atoms, TILE_WIDTH);
}

template <class T,u32 R>
void Tile<T,R>::CreateWindowAt(Point<int>& pt)
{
  m_executingWindow =  EventWindow<T,R>(pt, m_atoms, TILE_WIDTH);
}

template <class T,u32 R>
void Tile<T,R>::PlaceAtom(T& atom, Point<int>& pt)
{
  m_atoms[pt.GetX() + 
	  pt.GetY() * TILE_WIDTH] = atom;
}

template <class T,u32 R>
void Tile<T,R>::DiffuseAtom(EventWindow<T,R>& window)
{
  Point<int> neighbors[4];
  Point<int> center;
  ManhattanDir<R>::get().FillVNNeighbors(neighbors);
  u8 idx = rand() & 3;

  for(int i = 0; i < 4; i++)
  {
    idx++;
    idx &= 3;

    T& atom = window.GetRelativeAtom(neighbors[idx]);

    /* It's empty! Move there! */
    if(m_stateFunc(&atom) == 0)
    {
      Point<int> empty(0, 0);
      window.SwapAtoms(neighbors[idx], empty);
      return;
    }
  }
}

template <class T,u32 R>
void Tile<T,R>::Execute(ElementTable<T,R>& table)
{
  CreateRandomWindow();

  //Point<int> eventCenter(10, 8);
  //CreateWindowAt(eventCenter);
  
  table.Execute(m_executingWindow);

  DiffuseAtom(m_executingWindow);

  m_executingWindow.FillCenter(m_lastExecutedAtom);

  m_executingWindow.WriteTo(m_atoms, TILE_WIDTH);

  m_executingWindow.DeallocateAtoms();
}
