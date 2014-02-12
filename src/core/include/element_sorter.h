#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Sorter : public Element<T,R>
  {
  public:
    static Element_Sorter SORTER_INSTANCE;

    Element_Sorter() {}
    
    typedef u32 (* StateFunction )(T* atom);

    bool FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
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

    void FlipSEPointToCorner(Point<s32>& pt, EuclidDir corner)
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
    void FillSubwindowIndices(s32* indices,
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

    u32 FoundIndicesCount(s32* indices)
    {
      u32 count = 0;
      while(indices[count] != -1)
      {
	count++;
      }
      return count;
    }

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
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

    static void Needed();    
  };

  template <class T, u32 R>
  Element_Sorter<T,R> Element_Sorter<T,R>::SORTER_INSTANCE;

  template <class T, u32 R>
  void Element_Sorter<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_SORTER, &Element_Sorter<T,R>::SORTER_INSTANCE);
  }
}
