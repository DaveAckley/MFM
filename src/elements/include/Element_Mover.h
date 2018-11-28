#ifndef ELEMENT_MOVER_H   /* -*- C++ -*- */
#define ELEMENT_MOVER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "itype.h"
#include "FXP.h"

namespace MFM
{

  /**
    An element that moves other elements around and causes trouble for
    things that don't like that.  Occasionally converts Res

    Class:
    (C1) Name is Element_Mover
    (C2) Type is 0x140
   */


  template <class EC>
  class Element_Mover : public Element<EC>
  {
  public:
    virtual u32 GetTypeFromThisElement() const {
      return 0xCE0d;
    }
    
    enum { MOVER_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  public:

    static Element_Mover THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    static const u32 TYPE_BITS = 12;
    static const u32 TYPE_MASK = (1<<TYPE_BITS)-1;

    static const u32 STATE_BITS = 0;

    static bool IsMoverType(u32 type)
    {
      return type==TYPE();
    }

    Element_Mover() : Element<EC>(MFM_UUID_FOR("Mover", MOVER_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Mv");
      Element<EC>::SetName("Mover");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 GetElementColor() const
    {
      return 0xff0030e0;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      Random & random = window.GetRandom();

      T self = window.GetCenterAtomSym();
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

      const MDist<R> & md = MDist<R>::get();

      // Scan event window outside self
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(1); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        if (!window.IsLiveSiteSym(sp))
          continue;

        // Empty or occupied?
        const T other = window.GetRelativeAtomSym(sp);
        const u32 otherType = other.GetType();
        bool isEmpty = Element_Empty<EC>::THE_INSTANCE.IsType(otherType);

        if (isEmpty) {
          if (random.OneIn(++emptyCount)) {
            empty = sp;
          }
        } else {
          if (otherType == Element_Mover<EC>::THE_INSTANCE.GetType())
            ++moverCount;
          else if (otherType == Element_Dreg<EC>::THE_INSTANCE.GetType())
            ++dregCount;
          else if (otherType == Element_Res<EC>::THE_INSTANCE.GetType())
            ++resCount;

          if (random.OneIn(++occupCount)) {
            occup = sp;
          }
        }
      }

      if (occupCount > 0) {

        const T other = window.GetRelativeAtomSym(occup);
        const u32 otherType = other.GetType();
        bool isRes = otherType == Element_Res<EC>::TYPE();

        // Is it Res, and no other Movers, and we're inclined to convert?

        if (isRes && moverCount==0 && resCount > 1) {
          window.SetRelativeAtomSym(occup,self);  // Be Like Me!
        } else if (emptyCount > 0) {
          // If have empty and occup, swap
          window.SwapAtomsSym(occup,empty);
        }
      }

      window.Diffuse();
    }

  };

  template <class EC>
  Element_Mover<EC> Element_Mover<EC>::THE_INSTANCE;

}

#endif /* ELEMENT_MOVER_H */
