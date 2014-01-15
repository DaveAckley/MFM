#include <stdlib.h>
#include "elementtable.h"
#include "manhattandir.h"
#include "p1atom.h"

#define RAND_ONEIN(x) !(rand() % (x))
#define RAND_BOOL (rand() & 1)

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
    window.SetRelativeAtom(dir, T(newType));
  }
}

template <class T,u32 R>
void ElementTable<T,R>::SorterBehavior(EventWindow<T,R>& window,
				       StateFunction f)
{
  Point<int> dir;
  ManhattanDir<R>::get().FillRandomSingleDir(dir);

  T sorter = window.GetCenterAtom();

  u32 state = f(&window.GetRelativeAtom(dir));

  if(state == ELEMENT_RES)
  {
    window.SetRelativeAtom(dir, P1Atom(ELEMENT_SORTER));
  }

  u32 halfR = R >> 1;
  bool swapUp = RAND_BOOL;
  bool swapping = false;

  Point<int> rightSearchPtr(1, swapUp? 1 : halfR);
  Point<int> cpr;

  for(u32 x = 0; x < halfR; x++)
  {
    for(u32 y = 0; y < halfR; y++)
    {
      cpr.Set(x, y);
      cpr.Add(rightSearchPtr);
      
      T ratom = window.GetRelativeAtom(cpr);

      if(f(&ratom) == ELEMENT_DATA)
      {
	if(swapUp)
	{
	  if(sorter.ReadLowerBits() > ratom.ReadLowerBits())
	  {
	    swapping = true;
	    break;
	  }
	}
	else
	{
	  if(sorter.ReadLowerBits() < ratom.ReadLowerBits())
	  {
	    swapping = true;
	    break;
	  }
	}
      }
    }
    if(swapping)
    {
      break;
    }
  }
  
  if(swapping)
  {
    
  }
}

#define DATA_CREATE_ODDS 1000
#define DATA_MAXVAL 100
#define DATA_MINVAL 1

template <class T, u32 R>
void ElementTable<T,R>::EmitterBehavior(EventWindow<T,R>& window,
					StateFunction f)
{
  /* See if we can split up or down */

  Point<int> repPt = Point<int>(0, RAND_BOOL ? R : -R);
  if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
  {
    window.SetRelativeAtom(repPt, T(ELEMENT_EMITTER));
  }

  /* Create some data */
  
  ManhattanDir<R>::get().FillRandomSingleDir(repPt);

  if(RAND_ONEIN(DATA_CREATE_ODDS))
  {
    if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
    {
      T atom = T(ELEMENT_DATA);
      atom.WriteLowerBits(RAND_ONEIN(DATA_MAXVAL) + DATA_MINVAL);
      window.SetRelativeAtom(repPt, atom);
    }
  }
}


template <class T, u32 R>
void ElementTable<T,R>::ConsumerBehavior(EventWindow<T,R>& window,
					StateFunction f)
{
  
}

template <class T,u32 R>
ElementTable<T,R>::ElementTable(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
  m_funcmap[0] = &NothingBehavior;
  m_funcmap[1] = &DRegBehavior;
  m_funcmap[2] = &NothingBehavior;
  m_funcmap[3] = &SorterBehavior;
  m_funcmap[4] = &EmitterBehavior;
  m_funcmap[5] = &ConsumerBehavior;
  m_funcmap[6] = &NothingBehavior;
}

template <class T, u32 R>
bool ElementTable<T,R>::Diffusable(ElementType type)
{
  switch(type)
  {
  case ELEMENT_NOTHING:
  case ELEMENT_EMITTER:
  case ELEMENT_CONSUMER:
    return false;
  default:
    return true;
  }
}
