#include <stdlib.h> /* -*- C++ -*- */
#include "eucliddir.h"
#include "elementtable.h"
#include "manhattandir.h"
#include "p1atom.h"

namespace MFM {

template <class T,u32 R>
void ElementTable<T,R>::NothingBehavior(EventWindow<T,R>& window, StateFunction f)
{
  return;
}


#define DREG_RES_ODDS 20
#define DREG_DEL_ODDS 10
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 //Deleting DREGs

template <class T,u32 R>
void ElementTable<T,R>::DRegBehavior(EventWindow<T,R>& window, StateFunction f)
{
  Random & random = window.GetRandom();

  SPoint dir;
  ManhattanDir<R>::get().FillRandomSingleDir(dir);

  u32 state = f(&window.GetRelativeAtom(dir));

  ElementType newType = (ElementType)-1;

  if(state == ELEMENT_NOTHING)
  {
    if(random.OneIn(DREG_DRG_ODDS))
    {
      newType = ELEMENT_DREG;
    }
    else if(random.OneIn(DREG_RES_ODDS))
    {
      newType = ELEMENT_RES;
    }
  }
  else if(state == ELEMENT_DREG)
  {
    if(random.OneIn(DREG_DDR_ODDS))
    {
      newType = ELEMENT_NOTHING;
    }
  }
  else if(random.OneIn(DREG_DEL_ODDS))
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

template <class T, u32 R>
void ElementTable<T,R>::FlipSEPointToCorner(Point<s32>& pt, EuclidDir corner)
{
  switch(corner)
  {
  case EUDIR_SOUTHEAST: break;
  case EUDIR_NORTHEAST: ManhattanDir<R>::get().FlipAxis(pt, false); break;
  case EUDIR_SOUTHWEST: ManhattanDir<R>::get().FlipAxis(pt, true); break;
  case EUDIR_NORTHWEST:
    ManhattanDir<R>::get().FlipAxis(pt, true);
    ManhattanDir<R>::get().FlipAxis(pt, false);
    break;
  default: FAIL(ILLEGAL_ARGUMENT); break;
  }
}

/* Fills 'indices' with the indices of a Sub-windows of all       */
/* relative atoms which have type 'type' .                        */
/* Once all indices are found, a -1 is inserted, like a null      */
/* terminator.                                                    */
template <class T, u32 R>
void ElementTable<T,R>::FillSubwindowIndices(s32* indices,
					     EventWindow<T,R>& window,
					     StateFunction f,
					     ElementType type,
					     EuclidDir corner)
{
  /* As long as R is a power of two,             */
  /* ((R * R) / 4) is the size of one sub-window. */
  Point<s32> srcPt;
  for(u32 i = 0; i < ((R * R) / 4); i++)
  {
    srcPt = ManhattanDir<R>::get().GetSEWindowPoint(i);

    FlipSEPointToCorner(srcPt, corner);

    if(f(&window.GetRelativeAtom(srcPt)) == type)
    {
      *indices = ManhattanDir<R>::get().FromPoint(srcPt, MANHATTAN_TABLE_EVENT);
      indices++;
    }
  }
  *indices = -1;
}

template <class T ,u32 R>
u32 ElementTable<T,R>::FoundIndicesCount(s32* indices)
{
  u32 count = 0;
  while(indices[count] != -1)
  {
    count++;
  }
  return count;
}

template <class T,u32 R>
void ElementTable<T,R>::SorterBehavior(EventWindow<T,R>& window,
				       StateFunction f)
{
  Random & random = window.GetRandom();

  Point<s32> repPt;
  ManhattanDir<R>::get().FillRandomSingleDir(repPt);

  if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_RES)
  {
    window.SetRelativeAtom(repPt, T(ELEMENT_SORTER));
  }

  /* Add one for the terminator       v    */
  const u32 subSize = ((R * R) / 4) + 1;

  s32 seDatas[subSize];
  s32 neDatas[subSize];
  s32 nwEmpties[subSize];
  s32 swEmpties[subSize];

  FillSubwindowIndices(seDatas, window, f, ELEMENT_DATA, EUDIR_SOUTHEAST);
  FillSubwindowIndices(neDatas, window, f, ELEMENT_DATA, EUDIR_NORTHEAST);
  FillSubwindowIndices(swEmpties, window, f, ELEMENT_NOTHING, EUDIR_SOUTHWEST);
  FillSubwindowIndices(nwEmpties, window, f, ELEMENT_NOTHING, EUDIR_NORTHWEST);

  u32 seCount = FoundIndicesCount(seDatas);
  u32 neCount = FoundIndicesCount(neDatas);
  u32 swCount = FoundIndicesCount(swEmpties);
  u32 nwCount = FoundIndicesCount(nwEmpties);

  bool movingUp = random.CreateBool();
  
  Point<s32> srcPt, dstPt;
  for(s32 i = 0; i < 2; i++)
  {
    if(movingUp && seCount && nwCount)
    {
      ManhattanDir<R>::get().FillFromBits(srcPt, seDatas[random.Create(seCount)], MANHATTAN_TABLE_EVENT);
      ManhattanDir<R>::get().FillFromBits(dstPt, nwEmpties[random.Create(nwCount)], MANHATTAN_TABLE_EVENT);
    }
    else if(!movingUp && neCount && swCount)
    {
      ManhattanDir<R>::get().FillFromBits(srcPt, neDatas[random.Create(neCount)], MANHATTAN_TABLE_EVENT);
      ManhattanDir<R>::get().FillFromBits(dstPt, swEmpties[random.Create(swCount)], MANHATTAN_TABLE_EVENT);
    }
    else
    {
      movingUp = !movingUp;
      continue;
    }

    u32 cmp = ((movingUp && (window.GetRelativeAtom(srcPt).ReadLowerBits() >
			     window.GetCenterAtom().ReadLowerBits())) ||
	       (!movingUp && (window.GetRelativeAtom(srcPt).ReadLowerBits() <
			      window.GetCenterAtom().ReadLowerBits())));
    if(cmp)
    {
      window.GetCenterAtom().WriteLowerBits(window.GetRelativeAtom(srcPt).ReadLowerBits());
      window.SetRelativeAtom(dstPt, window.GetRelativeAtom(srcPt));
      window.SetRelativeAtom(srcPt, T(ELEMENT_NOTHING));
      return;
    }
  }
}

#define DATA_CREATE_ODDS 8
#define DATA_MAXVAL 100
#define DATA_MINVAL 1

template <class T, u32 R>
void ElementTable<T,R>::EmitterBehavior(EventWindow<T,R>& window,
					StateFunction f)
{
  Random & random = window.GetRandom();

  ReproduceVertically(window, f, ELEMENT_EMITTER);

  /* Create some data */
  SPoint repPt;
  ManhattanDir<R>::get().FillRandomSingleDir(repPt);

  if(random.OneIn(DATA_CREATE_ODDS))
  {
    if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
    {
      T atom = T(ELEMENT_DATA);
      atom.WriteLowerBits(random.Between(DATA_MINVAL, DATA_MAXVAL));
      window.SetRelativeAtom(repPt, atom);
    }
  }
}

template <class T, u32 R>
void ElementTable<T,R>::ReproduceVertically(EventWindow<T,R>& window, StateFunction f,
					    ElementType type)
{
  Random & random = window.GetRandom();

  u32 cval = window.GetCenterAtom().ReadLowerBits();
  bool down = random.CreateBool();
  SPoint repPt(0, down ? R/2 : -(R/2));
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
    printf("[%3d]Export!: %d sum %d\n", bnum, val, 3*bnum+val); // something sort of constant at equil.?
    window.SetRelativeAtom(consPt, T(ELEMENT_NOTHING));
  }
}

template <class T, u32 R>
void ElementTable<T,R>::SetStateFunction(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
}

template <class T,u32 R>
ElementTable<T,R>::ElementTable()
{
  m_funcmap[0] = &NothingBehavior;
  m_funcmap[1] = &DRegBehavior;
  m_funcmap[2] = &NothingBehavior;
  m_funcmap[3] = &SorterBehavior;
  m_funcmap[4] = &EmitterBehavior;
  m_funcmap[5] = &ConsumerBehavior;
  m_funcmap[6] = &NothingBehavior;
}

template <class T, u32 R>
ElementTable<T,R>& ElementTable<T,R>::get()
{
  static ElementTable<T,R> instance;
  return instance;
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

} /* namespace MFM */
