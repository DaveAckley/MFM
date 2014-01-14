#include <stdlib.h>
#include "elementtable.h"
#include "manhattandir.h"

#define RAND_ONEIN(x) !(rand() % (x))

template <class T,u32 R>
void ElementTable<T,R>::NothingBehavior(EventWindow<T,R>& window,
				      StateFunction f)
{
  return;
}

#define DREG_RES_ODDS 40
#define DREG_DEL_ODDS 20
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 /*Deleting DREGs*/



template <class T,u32 R>
void ElementTable<T,R>::DRegBehavior(EventWindow<T,R>& window,
				   StateFunction f)
{
  Point<int> dir;
  ManhattanDir<R>::get().FillRandomSingleDir(dir);

  u32 state = f(&window.GetRelativeAtom(dir));

  ElementType newType = (ElementType)-1;

  if(state == ELEMENT_NOTHING)
  {
    if(RAND_ONEIN(DREG_DRG_ODDS))
    {
      newType = ELEMENT_DREG;
    }
    else if(RAND_ONEIN(DREG_RES_ODDS))
    {
      newType = ELEMENT_RES;
    }
  }
  else if(state == ELEMENT_DREG)
  {
    if(RAND_ONEIN(DREG_DDR_ODDS))
    {
      newType = ELEMENT_NOTHING;
    }
  }
  else if(RAND_ONEIN(DREG_DEL_ODDS))
  {
    newType = ELEMENT_NOTHING;
  }

  if(newType >= 0)
  {
    window.SetRelativeAtom(dir, P1Atom(newType));
  }
}

template <class T,u32 R>
void ElementTable<T,R>::SorterBehavior(EventWindow<T,R>& window,
				       StateFunction f)
{
  Point<int> dir;
  ManhattanDir<R>::get().FillRandomSingleDir(dir);

  u32 state = f(&window.GetRelativeAtom(dir));

  if(state == ELEMENT_RES)
  {
    window.SetRelativeAtom(dir, P1Atom(ELEMENT_SORTER));
  }
}

template <class T,u32 R>
ElementTable<T,R>::ElementTable(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
  m_funcmap[0] = &NothingBehavior;
  m_funcmap[1] = &DRegBehavior;
  m_funcmap[2] = &NothingBehavior;
  m_funcmap[3] = &SorterBehavior;
}

template <class T, u32 R>
bool ElementTable<T,R>::Diffusable(ElementType type)
{
  switch(type)
  {
  case ELEMENT_NOTHING:
    return false;
  default:
    return true;
  }
}
