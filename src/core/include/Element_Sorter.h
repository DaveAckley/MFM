#ifndef ELEMENT_SORTER_H       /* -*- C++ -*- */
#define ELEMENT_SORTER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Res.h"  /* For Element_Res::TYPE */
#include "Element_Data.h"  /* For Element_Data::TYPE */
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Sorter : public Element<T,R>
  {
  public:
    const char* GetName() const { return "Sorter"; }

    static Element_Sorter THE_INSTANCE;
    static const u32 TYPE = 0xab;                // We compare a vs b
    static const u32 STATE_THRESHOLD_IDX = 0;    // First bit in state
    static const u32 STATE_THRESHOLD_LEN = 32;
    static const u32 STATE_BITS = STATE_THRESHOLD_LEN;

    static const SPoint m_southeastSubwindow[4];

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
				     SPoint& pt, Dir subwindow, ElementType type) const
    {
      return this->FillPointWithType(window, pt, 
                                     m_southeastSubwindow,
                                     sizeof(m_southeastSubwindow)/sizeof(m_southeastSubwindow[0]),
                                     subwindow, type);
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xffff0000;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      Random & random = window.GetRandom();
      SPoint reproducePt;
      if(this->FillPointWithType(window, reproducePt,
                                 Element<T,R>::VNNeighbors, 4, Dirs::SOUTHEAST, Element_Res<T,R>::TYPE))
      {
        T newAtom = this->GetDefaultAtom();
        u32 myThresh = GetThreshold(window.GetCenterAtom(),0);
        SetThreshold(newAtom,myThresh);
	window.SetRelativeAtom(reproducePt, newAtom);
      }

      SPoint seData, neData, swEmpty, nwEmpty, srcPt, dstPt;
      bool movingUp = random.CreateBool();
  
      for(s32 i = 0; i < 2; i++)
      {
	if(movingUp &&
	   FillAvailableSubwindowPoint(window, seData, Dirs::SOUTHEAST, Element_Data<T,R>::TYPE) &&
	   FillAvailableSubwindowPoint(window, nwEmpty, Dirs::NORTHWEST, ELEMENT_EMPTY))
	{
	  srcPt = seData;
	  dstPt = nwEmpty;
	}
	else if(!movingUp &&
                FillAvailableSubwindowPoint(window, neData, Dirs::NORTHEAST, Element_Data<T,R>::TYPE) &&
                FillAvailableSubwindowPoint(window, swEmpty, Dirs::SOUTHWEST, ELEMENT_EMPTY))
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
          T ctr = window.GetCenterAtom();
          SetThreshold(ctr,datum);
          window.SetCenterAtom(ctr);
	  window.SwapAtoms(srcPt, dstPt);
	  return;
	}
      }
      this->Diffuse(window);
    }

    static void Needed();

  };

  template <class T, u32 R>
  Element_Sorter<T,R> Element_Sorter<T,R>::THE_INSTANCE;

  template <class T, u32 R>
  const SPoint Element_Sorter<T,R>::m_southeastSubwindow[4] = {
    SPoint(1,1),SPoint(1,2),SPoint(2,1),SPoint(2,2)
  };

  /*
  template <class T, u32 R>
  void Element_Sorter<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Sorter<T,R>::THE_INSTANCE);
  }
  */
}
#endif /* ELEMENT_SORTER_H */
