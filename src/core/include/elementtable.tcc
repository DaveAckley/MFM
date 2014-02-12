#include <stdlib.h> /* -*- C++ -*- */
#include "eucliddir.h"
#include "elementtable.h"
#include "manhattandir.h"
#include "p1atom.h"

namespace MFM {

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
void ElementTable<T,R>::SetStateFunction(u32 (*stateFunc)(T* atom))
{
  m_statefunc = stateFunc;
}

template <class T,u32 R>
ElementTable<T,R>::ElementTable()
{
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
