#include <stdlib.h> /* -*- C++ -*- */
#include "eucliddir.h"
#include "elementtable.h"
#include "manhattandir.h"
#include "p1atom.h"

#define RAND_ONEIN(x) !(rand() % (x))
#define RAND_BETWEEN(x, y) ((x) + (rand() % (y)))
#define RAND_MOD(x) (rand() % (x))
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
    u32 newState = sorter.ReadLowerBits();
    switch(EuDir::FromOffset(dir))
    {
    case EUDIR_NORTH: newState -= 5; break;
    case EUDIR_SOUTH: newState += 5; break;
    default: break;
    }
    window.SetRelativeAtom(dir, P1Atom(ELEMENT_SORTER));
    window.GetRelativeAtom(dir).WriteLowerBits(newState);
  }

  u32 halfR = R >> 1;
  bool swapUp = RAND_BOOL;
  bool swapping = false;

  Point<int> rightSearchPtr(1, swapUp? 1 : -halfR);
  Point<int> srcPt;

  for(u32 x = 0; x < halfR; x++)
  {
    for(u32 y = 0; y < halfR; y++)
    {
      srcPt.Set(x, y);
      srcPt.Add(rightSearchPtr);
      
      T ratom = window.GetRelativeAtom(srcPt);

      if(f(&ratom) == ELEMENT_DATA)
      {
	if(swapUp)
	{
	  if(sorter.ReadLowerBits() > ratom.ReadLowerBits())
	  {
	    sorter.WriteLowerBits(ratom.ReadLowerBits());
	    swapping = true;
	    break;
	  }
	}
	else
	{
	  if(sorter.ReadLowerBits() < ratom.ReadLowerBits())
	  {
	    sorter.WriteLowerBits(ratom.ReadLowerBits());
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

  rightSearchPtr = Point<s32>(-halfR, swapUp? -halfR : 1);
  Point<s32> destPt;
  
  if(swapping)
  {
    for(u32 x = 0; x < halfR; x++)
    {
      for(u32 y = 0; y < halfR; y++)
      {
	destPt.Set(x, y);
	destPt.Add(rightSearchPtr);

	if(f(&window.GetRelativeAtom(destPt)) == ELEMENT_NOTHING)
	{
	  if(window.SetRelativeAtom(destPt,
				    window.GetRelativeAtom(srcPt)))
	  {
	    window.SetRelativeAtom(srcPt, T(ELEMENT_NOTHING));
	  }
	}
      }
    }
  }
}

#define DATA_CREATE_ODDS 25
#define DATA_MAXVAL 100
#define DATA_MINVAL 1

template <class T, u32 R>
void ElementTable<T,R>::EmitterBehavior(EventWindow<T,R>& window,
					StateFunction f)
{
  ReproduceVertically(window, f, ELEMENT_EMITTER);

  /* Create some data */
  Point<int> repPt;
  ManhattanDir<R>::get().FillRandomSingleDir(repPt);

  if(RAND_ONEIN(DATA_CREATE_ODDS))
  {
    if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
    {
      T atom = T(ELEMENT_DATA);
      atom.WriteLowerBits(RAND_BETWEEN(DATA_MINVAL, DATA_MAXVAL));
      window.SetRelativeAtom(repPt, atom);
    }
  }
}

template <class T, u32 R>
void ElementTable<T,R>::ReproduceVertically(EventWindow<T,R>& window,
					    StateFunction f,
					    ElementType type)
{
  u32 cval = window.GetCenterAtom().ReadLowerBits();
  bool down = RAND_BOOL;
  Point<int> repPt = Point<int>(0, down ? R : -R);
  if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
  {
    window.SetRelativeAtom(repPt, T(type));
    window.GetRelativeAtom(repPt).WriteLowerBits(cval + (down ? 1 : -1));
  }
}


template <class T, u32 R>
void ElementTable<T,R>::ConsumerBehavior(EventWindow<T,R>& window,
					StateFunction f)
{
  ReproduceVertically(window, f, ELEMENT_CONSUMER);

  Point<s32> consPt;
  ManhattanDir<R>::get().FillRandomSingleDir(consPt);

  if(f(&window.GetRelativeAtom(consPt)) == ELEMENT_DATA)
  {
    u32 val = window.GetRelativeAtom(consPt).ReadLowerBits();
    u32 bnum = window.GetCenterAtom().ReadLowerBits();
    printf("[%3d]Export!: %d\n", bnum, val);
    window.SetRelativeAtom(consPt, T(ELEMENT_NOTHING));
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
