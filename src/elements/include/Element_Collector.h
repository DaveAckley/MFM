#ifndef ELEMENT_COLLECTOR_H   /* -*- C++ -*- */
#define ELEMENT_COLLECTOR_H

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
    An element that tries to sort other elements into vague piles
    based on their types.
   */


  template <class CC>
  class Element_Collector : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    enum {
      COLLECTOR_VERSION = 1
    };

    static Element_Collector THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    static bool IsMyType(u32 type) {
      return type==TYPE();
    }

    Element_Collector() : Element<CC>(MFM_UUID_FOR("PileMaker", COLLECTOR_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Pm");
      Element<CC>::SetName("Collector (PileMaker)");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff00c8c8;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff007070;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      T self = window.GetCenterAtom();
      const u32 selfType = self.GetType();

      if (!IsMyType(selfType))
        FAIL(ILLEGAL_STATE);

      SPoint occup;
      u32 occupCount = 0;
      u32 occupType = 0;

      u32 meCount = 0;
      SPoint resLoc;
      u32 resCount = 0;

      SPoint empty;
      u32 emptyCount = 0;

      const MDist<R> & md = MDist<R>::get();

      // Scan near me for mytypes, res, or other object
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(2); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        if (!window.IsLiveSite(sp))
          continue;

        // Empty or occupied?
        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();
        bool isEmpty = Element_Empty<CC>::THE_INSTANCE.IsType(otherType);

        if (isEmpty) {
          if (random.OneIn(++emptyCount)) {
            empty = sp;
          }
        } else {
          if (otherType == selfType)
            ++meCount;
          else if (otherType == Element_Res<CC>::THE_INSTANCE.GetType()) {
            if (random.OneIn(++resCount)) {
              resLoc = sp;
            }
          } else if (random.OneIn(++occupCount)) {
            occup = sp;
            occupType = otherType;
          }
        }
      }

      if (occupCount > 0 && emptyCount > 0) {

        u32 otherCount = 0;

        // Scan near them for other theirtypes
        for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(2); ++idx) {
          const SPoint sp = md.GetPoint(idx) + occup;

          if (!window.IsLiveSite(sp))
            continue;

          const T other = window.GetRelativeAtom(sp);
          const u32 otherType = other.GetType();

          // One of them?  One of them?
          if (otherType == occupType) {
            ++otherCount;
          }
        }

        if ((otherCount == 0 || random.OneIn(100*otherCount)) && emptyCount > 0) {
          // They are alone (or unlucky) and we have an empty nearish us.  Pull them in
          window.SwapAtoms(occup,empty);
        } else {

          // Nothing to do.  Clonebomb?
          if (resCount > 1 && meCount == 0) {
            window.SetRelativeAtom(resLoc,self);  // Be Like Me!
          }
        }
      }

      window.Diffuse();
    }

  };

  template <class CC>
  Element_Collector<CC> Element_Collector<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_COLLECTOR_H */
