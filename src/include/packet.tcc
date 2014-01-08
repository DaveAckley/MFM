template <class T>
Packet<T>::Packet(PacketType type)
{
  m_type = type;
}


template <class T>
void Packet<T>::SetAtom(T& atom)
{
  m_atom = atom;
}

template <class T>
T& Packet<T>::GetAtom()
{
  return m_atom;
}

template <class T>
void Packet<T>::SetLocation(Point<int>& fromPt)
{
  m_edgeLoc = Point<int>(fromPt);
}

template <class T>
Point<int>& Packet<T>::GetLocation()
{
  return m_edgeLoc;
}

template <class T>
void Packet<T>::SetEdge(EuclidDir edge)
{
  m_edge = edge;
}

template <class T>
EuclidDir Packet<T>::GetEdge()
{
  return m_edge;
}
