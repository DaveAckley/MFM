template <class T>
Point<T>::Point(T x, T y)
{
  m_x = x;
  m_y = y;
}

template <class T>
Point<T>::Point(bool random)
{
  if(random)
  {
    m_x = (T)rand();
    m_y = (T)rand();
  }
  else
  {
    m_x = 0;
    m_y = 0;
  }
}

template <class T>
T Point<T>::GetX()
{
  return m_x;
}

template <class T>
T Point<T>::GetY()
{
  return m_y;
}

template <class T>
void Point<T>::Add(Point<T>* offset)
{
  m_x += offset->m_x;
  m_y += offset->m_y;
}

template <class T>
void Point<T>::Add(T x, T y)
{
  m_x += x;
  m_y += y;
}

template <class T>
void Point<T>::Multiply(T scalar)
{
  m_x *= scalar;
  m_y *= scalar;
}

template <class T>
void Point<T>::SetX(T x)
{
  m_x = x;
}

template <class T>
void Point<T>::SetY(T y)
{
  m_y = y;
}
