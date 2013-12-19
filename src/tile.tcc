template <class T>
Tile<T>::Tile(Point<int>* location)
{
  m_location.SetX(location->GetX());
  m_location.SetY(location->GetY());
}

template <class T>
Tile<T>::Tile(int x, int y)
{
  m_location.SetX(x);
  m_location.SetY(y);
}

template <class T>
Tile<T>::~Tile()
{

}

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
EventWindow* Tile<T>::CreateWindow(Point<int>* ctr)
{
  return new EventWindow(ctr, m_atoms);
}

template <class T>
EventWindow* Tile<T>::CreateRandomWindow()
{
  
}
