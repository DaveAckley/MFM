#ifndef ELEMENT_SORTER_H
#define ELEMENT_SORTER_H

#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "element_res.h"  /* For Element_Res::TYPE */
#include "element_data.h"  /* For Element_Data::TYPE */
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Sorter : public Element<T,R>
  {
  public:
    static Element_Sorter THE_INSTANCE;
    static const u32 TYPE = 0xab;                // We compare a vs b
    static const u32 STATE_THRESHOLD_IDX = 0;    // First bit in state
    static const u32 STATE_THRESHOLD_LEN = 32;
    static const u32 STATE_BITS = STATE_THRESHOLD_LEN;

    Element_Sorter() { }
    
    u32 GetThreshold(const T &atom, u32 badType) const {
      if (!atom.IsType(TYPE)) return badType;
      return atom.GetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN);
    }

    bool SetThreshold(T &atom, u32 value) const {
      if (!atom.IsType(TYPE)) return false;
      atom.SetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN,value);
      return true;
    }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    bool FillAvailableSubwindowPoint(EventWindow<T,R>& window, 
				     SPoint& pt, EuclidDir subwindow, ElementType type) const
    {
      return FillPointWithType(window, pt, 
			       ManhattanDir<R>::get().GetSESubWindow(), 
			       ((R*R)/4), subwindow, type);
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xffff0000;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      Random & random = window.GetRandom();
      SPoint reproducePt;
      if(FillPointWithType(window, reproducePt,
			   Element<T,R>::VNNeighbors, 4, EUDIR_SOUTHEAST, Element_Res<T,R>::TYPE))
      {
	window.SetRelativeAtom(reproducePt, this->GetDefaultAtom());
      }

      SPoint seData, neData, swEmpty, nwEmpty, srcPt, dstPt;
      bool movingUp = random.CreateBool();
  
      for(s32 i = 0; i < 2; i++)
      {
	if(movingUp &&
	   FillAvailableSubwindowPoint(window, seData, EUDIR_SOUTHEAST, Element_Data<T,R>::TYPE) &&
	   FillAvailableSubwindowPoint(window, nwEmpty, EUDIR_NORTHWEST, ELEMENT_NOTHING))
	{
	  srcPt = seData;
	  dstPt = nwEmpty;
	}
	else if(!movingUp &&
                FillAvailableSubwindowPoint(window, neData, EUDIR_NORTHEAST, Element_Data<T,R>::TYPE) &&
                FillAvailableSubwindowPoint(window, swEmpty, EUDIR_SOUTHWEST, ELEMENT_NOTHING))
	{
	  srcPt = neData;
	  dstPt = swEmpty;
	}
	else
	{
	  movingUp = !movingUp;
	  continue;
	}
	
        u32 threshold = GetThreshold(window.GetCenterAtom(),0);
        u32 datum = Element_Data<T,R>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(srcPt),0);
	u32 cmp = (movingUp && (datum > threshold)) || (!movingUp && (datum < threshold));
	if(cmp)
	{
	  SetThreshold(window.GetCenterAtom(),datum);
	  window.SwapAtoms(srcPt, dstPt);
	  return;
	}
      }
      Diffuse(window);
    }

    static void Needed();

  };

  template <class T, u32 R>
  Element_Sorter<T,R> Element_Sorter<T,R>::THE_INSTANCE;

  /*
  template <class T, u32 R>
  void Element_Sorter<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Sorter<T,R>::THE_INSTANCE);
  }
  */
}
#endif /* ELEMENT_SORTER_H */
