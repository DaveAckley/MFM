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

template <class T, u32 R>
bool ElementTable<T,R>::FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
						ElementType type, StateFunction f, 
						EuclidDir corner)
{
  u32 startIdx = rand() % R;
  
  Point<s32> srcPt;
  for(u32 i = 0; i < R; i++)
  {
    srcPt = ManhattanDir<R>::get().GetSEWindowPoint((startIdx + i) % R);

    switch(corner)
    {
    case EUDIR_SOUTHEAST: break;
    case EUDIR_NORTHEAST: ManhattanDir<R>::get().FlipAxis(srcPt, false); break;
    case EUDIR_SOUTHWEST: ManhattanDir<R>::get().FlipAxis(srcPt, true); break;
    case EUDIR_NORTHWEST:
      ManhattanDir<R>::get().FlipAxis(srcPt, true);
      ManhattanDir<R>::get().FlipAxis(srcPt, false);
      break;
    default: FAIL(ILLEGAL_ARGUMENT); break;
    }

    if(f(&window.GetRelativeAtom(srcPt)) == type)
    {
      pt.Set(srcPt.GetX(), srcPt.GetY());
      return true;
    }
  }

  return false;
}

template <class T,u32 R>
void ElementTable<T,R>::SorterBehavior(EventWindow<T,R>& window,
				       StateFunction f)
{
  Point<s32> repPt;
  ManhattanDir<R>::get().FillRandomSingleDir(repPt);

  if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_RES)
  {
    window.SetRelativeAtom(repPt, T(ELEMENT_SORTER));
  }

  EuclidDir firstEdge = RAND_BOOL ? EUDIR_SOUTHEAST : EUDIR_NORTHEAST;
  EuclidDir firstOpposite = firstEdge == EUDIR_SOUTHEAST ? EUDIR_NORTHWEST :
    EUDIR_SOUTHWEST;
  EuclidDir lastEdge = firstEdge == EUDIR_SOUTHEAST ? EUDIR_NORTHEAST :
    EUDIR_SOUTHEAST;
  EuclidDir lastOpposite = firstEdge == EUDIR_SOUTHEAST ? EUDIR_NORTHWEST :
    EUDIR_SOUTHWEST;

  Point<s32> srcPt;
  Point<s32> dstPt;
  for(u32 i = 0; i < R; i++)
  {
    if(FillSubWindowContaining(srcPt, window, ELEMENT_DATA, f,
			       firstEdge) &&
       FillSubWindowContaining(dstPt, window, ELEMENT_NOTHING, f,
			       firstOpposite))
    {
      if(window.GetRelativeAtom(srcPt).ReadLowerBits() <
	 window.GetCenterAtom().ReadLowerBits())
      {
	window.GetCenterAtom().WriteLowerBits(window.GetRelativeAtom(srcPt).ReadLowerBits());
	window.SetRelativeAtom(dstPt, window.GetRelativeAtom(srcPt));
	window.SetRelativeAtom(srcPt, T(ELEMENT_NOTHING));
      }
    }
    else if(FillSubWindowContaining(srcPt, window, ELEMENT_DATA, f,
				    lastEdge) &&
	    FillSubWindowContaining(dstPt, window, ELEMENT_NOTHING, f,
				    lastOpposite))
    {
      if(window.GetRelativeAtom(srcPt).ReadLowerBits() >
	 window.GetCenterAtom().ReadLowerBits())
      {
	window.GetCenterAtom().WriteLowerBits(window.GetRelativeAtom(srcPt).ReadLowerBits());
	window.SetRelativeAtom(dstPt, window.GetRelativeAtom(srcPt));
	window.SetRelativeAtom(srcPt, T(ELEMENT_NOTHING));
      }
    }
  }
}

#define DATA_CREATE_ODDS 5
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
