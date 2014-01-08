template <class T>
Point<T>::Point(T x, T y)
{
  m_x = x;
  m_y = y;
}

template <class T>
Point<T>::Point(const Point<T> & p)
{
  m_x = p.m_x;
  m_y = p.m_y;
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
    m_x = m_y = 0;
  }
}

template <class T>
Point<T>::Point(bool random, T maxX, T maxY)
{
  if(random)
  {
    m_x = (T)rand() % maxX;
    m_y = (T)rand() % maxY;
  }
  else
  {
    m_x = m_y = 0;
  }
}

template <class T>
T Point<T>::GetX() const
{
  return m_x;
}

template <class T>
T Point<T>::GetY() const
{
  return m_y;
}

template <class T>
u32 Point<T>::GetManhattanDistance() const
{
  return (u32) (abs(m_x) + abs(m_y));
}

template <class T>
double Point<T>::GetLength() const
{
  return sqrt(m_x * m_x + m_y * m_y);
}

template <class T>
void Point<T>::Add(const Point<T>& offset)
{
  m_x += offset.m_x;
  m_y += offset.m_y;
}

template <class T>
void Point<T>::Add(T x, T y)
{
  m_x += x;
  m_y += y;
}

template <class T>
void Point<T>::Subtract(const Point<T>& offset)
{
  m_x -= offset.m_x;
  m_y -= offset.m_y;
}

template <class T>
void Point<T>::Subtract(T x, T y)
{
  m_x -= x;
  m_y -= y;
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

template <class T>
inline bool operator==(Point<T>& lhs, Point<T>& rhs)
{
  return lhs.GetX() == rhs.GetX() &&
    lhs.GetY() == rhs.GetY();
}

template <class T>
void Point<T>::Set(T x, T y)
{
  m_x = x;
  m_y = y;
}

template <class T>
Point<T>& Point<T>::operator=(const Point<T>& rhs)
{
  m_x = rhs.m_x;
  m_y = rhs.m_y;
  return *this;
}
