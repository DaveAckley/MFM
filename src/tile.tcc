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
  /* Make sure not to be created in the cache */
  int maxval = TILE_WIDTH - (EVENT_WINDOW_RADIUS << 1);
  Point<int> pt(true, maxval, maxval);
  pt.Add(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);

  return new EventWindow<T>(pt, m_atoms, maxval);
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
void Tile<T>::Execute(ElementTable<T>& table)
{
  EventWindow<T>* window = CreateRandomWindow();
  
  table.Execute(*window);
  
  delete window;
}
