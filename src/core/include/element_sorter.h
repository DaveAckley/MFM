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

    bool FillAvailableSubwindowPoint(EventWindow<T,R>& window, StateFunction f,
				     SPoint& pt, EuclidDir subwindow, ElementType type)
    {
      return FillPointWithType(window, f, pt, 
			       ManhattanDir<R>::get().GetSESubWindow(), 
			       ((R*R)/4), subwindow, type);
    }

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      Random & random = window.GetRandom();
      SPoint reproducePt;
      if(FillPointWithType(window, f, reproducePt,
			   Element<T,R>::VNNeighbors, 4, EUDIR_SOUTHEAST, ELEMENT_RES))
      {
	window.SetRelativeAtom(reproducePt, T(ELEMENT_SORTER));
      }

      SPoint seData, neData, swEmpty, nwEmpty, srcPt, dstPt;
      bool movingUp = random.CreateBool();
  
      for(s32 i = 0; i < 2; i++)
      {
	if(movingUp &&
	   FillAvailableSubwindowPoint(window, f, seData, EUDIR_SOUTHEAST, ELEMENT_DATA) &&
	   FillAvailableSubwindowPoint(window, f, nwEmpty, EUDIR_NORTHWEST, ELEMENT_NOTHING))
	{
	  srcPt = seData;
	  dstPt = nwEmpty;
	}
	else if(!movingUp &&
	   FillAvailableSubwindowPoint(window, f, neData, EUDIR_NORTHEAST, ELEMENT_DATA) &&
	   FillAvailableSubwindowPoint(window, f, swEmpty, EUDIR_SOUTHWEST, ELEMENT_NOTHING))
	{
	  srcPt = neData;
	  dstPt = swEmpty;
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
	  window.SwapAtoms(srcPt, dstPt);
	  return;
	}
      }
      Diffuse(window, f);
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
