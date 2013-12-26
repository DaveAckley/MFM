#include "elementtable.hpp"

template <class T>
void ElementTable<T>::NothingBehavior(EventWindow<T>& window)
{
  return;
}

template <class T>
void ElementTable<T>::DRegBehavior(EventWindow<T>& window)
{
  
}

template <class T>
ElementTable<T>::ElementTable(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
  m_funcmap[0] = &NothingBehavior;
}
