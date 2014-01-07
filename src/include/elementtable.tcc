#include <stdlib.h>
#include "elementtable.h"
#include "manhattandir.h"

#define RAND_ONEIN(x) !(rand() % (x))

template <class T>
void ElementTable<T>::NothingBehavior(EventWindow<T>& window,
				      StateFunction f)
{
  return;
}

#define DREG_RES_ODDS 40
#define DREG_DEL_ODDS 20
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 /*Deleting DREGs*/



template <class T>
void ElementTable<T>::DRegBehavior(EventWindow<T>& window,
				   StateFunction f)
{
  Point<int> dir;
  ManhattanDir::FillRandomSingleDir(dir);

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

template <class T>
ElementTable<T>::ElementTable(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
  m_funcmap[0] = &NothingBehavior;
  m_funcmap[1] = &DRegBehavior;
  m_funcmap[2] = &NothingBehavior;
}
