#ifndef ELEMENT_SORTER_H       /* -*- C++ -*- */
#define ELEMENT_SORTER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Res.h"  /* For Element_Res::TYPE */
#include "Element_Data.h"  /* For Element_Data::TYPE */
#include "ColorMap.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define SORTER_VERSION 1

  template <class CC>
  class Element_Sorter : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:

    static Element_Sorter THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    static const u32 STATE_THRESHOLD_IDX = 0;    // First bit in state
    static const u32 STATE_THRESHOLD_LEN = 32;
    static const u32 STATE_BITS = STATE_THRESHOLD_LEN;

    static const SPoint m_southeastSubwindow[4];

    Element_Sorter() : Element<CC>(MFM_UUID_FOR("Sorter", SORTER_VERSION)) { }

    u32 GetThreshold(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,TYPE())) return badType;
      return atom.GetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN);
    }

    bool SetThreshold(T &atom, u32 value) const {
      if (!Atom<CC>::IsType(atom,TYPE())) return false;
      atom.SetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN,value);
      return true;
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const
    {
      return 100;
    }

    bool FillAvailableSubwindowPoint(EventWindow<CC>& window,
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

    virtual u32 LocalPhysicsColor(const T & atom, u32 selector) const
    {
      switch (selector) {
      case 1:
        return 0xff503030;
        //        return ColorMap_SEQ5_Greys::THE_INSTANCE.
        //          GetInterpolatedColor(GetThreshold(atom,0),DATA_MINVAL,DATA_MAXVAL,0xffff0000);
      default:
        return DefaultPhysicsColor();
      }
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();
      SPoint reproducePt;
      T self = window.GetCenterAtom();
      if(this->FillPointWithType(window, reproducePt,
                                 Element<CC>::VNNeighbors, 4, Dirs::SOUTHEAST, Element_Res<CC>::TYPE()))
      {
	window.SetRelativeAtom(reproducePt, self);
      }

      SPoint seData, neData, swEmpty, nwEmpty, srcPt, dstPt;
      bool movingUp = random.CreateBool();

      for(s32 i = 0; i < 2; i++)
      {
	if(movingUp &&
	   FillAvailableSubwindowPoint(window, seData, Dirs::SOUTHEAST, Element_Data<CC>::TYPE()) &&
	   FillAvailableSubwindowPoint(window, nwEmpty, Dirs::NORTHWEST, Element_Empty<CC>::TYPE()))
	{
	  srcPt = seData;
	  dstPt = nwEmpty;
	}
	else if(!movingUp &&
                FillAvailableSubwindowPoint(window, neData, Dirs::NORTHEAST, Element_Data<CC>::TYPE()) &&
                FillAvailableSubwindowPoint(window, swEmpty, Dirs::SOUTHWEST, Element_Empty<CC>::TYPE()))
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
        u32 datum = Element_Data<CC>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(srcPt),0);
	u32 cmp = (movingUp && (datum < threshold)) || (!movingUp && (datum > threshold));
	if(cmp)
	{
          SetThreshold(self,datum);
          window.SetCenterAtom(self);
	  window.SwapAtoms(srcPt, dstPt);
          //	  return;
          break;
	}
      }
      this->Diffuse(window);
    }
  };

  template <class CC>
  Element_Sorter<CC> Element_Sorter<CC>::THE_INSTANCE;

  template <class CC>
  const SPoint Element_Sorter<CC>::m_southeastSubwindow[4] = {
    SPoint(1,1),SPoint(1,2),SPoint(2,1),SPoint(2,2)
  };

}
#endif /* ELEMENT_SORTER_H */
