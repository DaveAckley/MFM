#ifndef ELEMENT_MOVER_H   /* -*- C++ -*- */
#define ELEMENT_MOVER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Res.h"
#include "itype.h"
#include "FXP.h"
#include "P1Atom.h"

namespace MFM
{

  /**
    An element that moves other elements around and causes trouble for
    things that don't like that.  Occasionally converts Res

    Class:
    (C1) Name is Element_Mover
    (C2) Type is 0x140
   */


  template <class CC>
  class Element_Mover : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    const char* GetName() const { return "Mover"; }

    static Element_Mover THE_INSTANCE;

    static const u32 TYPE = 0x140;
    static const u32 TYPE_BITS = 12;
    static const u32 TYPE_MASK = (1<<TYPE_BITS)-1;

    static const u32 STATE_BITS = 0;

    static bool IsMoverType(u32 type) {
      return type==TYPE;
    }

    Element_Mover() { }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff0030e0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      T self = window.GetCenterAtom();
      const u32 selfType = self.GetType();

      if (!IsMoverType(selfType))
        FAIL(ILLEGAL_STATE);

      SPoint occup;
      u32 occupCount = 0;

      u32 moverCount = 0;
      u32 dregCount = 0;
      u32 resCount = 0;

      SPoint empty;
      u32 emptyCount = 0;

      const MDist<R> md = MDist<R>::get();

      // Scan event window outside self
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(1); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        // Empty or occupied?
        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();
        bool isEmpty = otherType == Element_Empty<CC>::TYPE;

        if (isEmpty) {
          if (random.OneIn(++emptyCount)) {
            empty = sp;
          }
        } else {
          if (otherType == Element_Mover<CC>::TYPE)
            ++moverCount;
          else if (otherType == Element_Dreg<CC>::TYPE)
            ++dregCount;
          else if (otherType == Element_Res<CC>::TYPE)
            ++resCount;

          if (random.OneIn(++occupCount)) {
            occup = sp;
          }
        }
      }

      if (occupCount > 0) {

        const T other = window.GetRelativeAtom(occup);
        const u32 otherType = other.GetType();
        bool isRes = otherType == Element_Res<CC>::TYPE;

        // Is it Res, and no other Movers, and we're inclined to convert?

        if (isRes && moverCount==0 && resCount > 1) {
          window.SetRelativeAtom(occup,self);  // Be Like Me!
        } else if (emptyCount > 0) {
          // If have empty and occup, swap
          window.SwapAtoms(occup,empty);
        }
      }

      this->Diffuse(window);
    }

  };

  template <class CC>
  Element_Mover<CC> Element_Mover<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_MOVER_H */
