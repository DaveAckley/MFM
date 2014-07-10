#ifndef ELEMENT_BAR_H   /* -*- C++ -*- */
#define ELEMENT_BAR_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Res.h"
#include "itype.h"
#include "FXP.h"
#include "P1Atom.h"
#include "ColorMap.h"

namespace MFM
{

  /**
    A bar-forming class.

    Bar grid positions in an event window

       -4 -3 -2 -1  0  1  2  3  4
    -4  .  .  .  .  x  .  .  .  .
    -3  .  .  .  .  .  .  .  .  .
    -2  .  .  x  .  x  .  x  .  .
    -1  .  .  .  .  .  .  .  .  .
     0  x  .  x  .  C  .  x  .  x
     1  .  .  .  .  .  .  .  .  .
     2  .  .  x  .  x  .  x  .  .
     3  .  .  .  .  .  .  .  .  .
     4  .  .  .  .  x  .  .  .  .

    Each bar atom knows how big the grid is supposed to be
    (GetMax()), and where within that grid it is supposed to be
    (GetPos()).


    Class:
    (C1) Name is Element_Bar
    (C2) Type is 0xba2
   */


  template <class CC>
  class Element_Bar : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

    template <u32 TVBITS>
    static u32 toSignMag(s32 value) {
      const u32 SIGN_BIT = 1<<(TVBITS-1);
      const u32 MAX = SIGN_BIT-1;
      u32 sign = 0;
      if (value < 0) {
        sign = SIGN_BIT;
        value = -value;
      }
      if (value > MAX)
        value = MAX;
      u32 val = (u32) value;
      return sign|val;
    }

    template <u32 TVBITS>
    static u32 toMag(u32 value) {
      const u32 MAX = (1<<TVBITS)-1;
      if (value < 0) {
        FAIL(ILLEGAL_ARGUMENT);
      }
      if (value > MAX)
        value = MAX;
      u32 val = value;
      return val;
    }

    template <u32 TVBITS>
    static s32 fromSignMag(const u32 value) {
      const u32 SIGN_BIT = 1<<(TVBITS-1);
      const u32 MASK = SIGN_BIT-1;
      FXP16 val = value&MASK;
      if (value & SIGN_BIT)
        val = -val;
      return val;
    }

    template <u32 TVBITS>
    static u32 fromMag(const u32 value) {
      const u32 MASK = (1<<TVBITS)-1;
      u32 val = value&MASK;
      return val;
    }

    template <u32 TVBITS>
    static u32 toSTiny(const SPoint & v) {
      u32 x = toSignMag<TVBITS>(v.GetX());
      u32 y = toSignMag<TVBITS>(v.GetY());
      return (x<<TVBITS)|y;
    }


    template <u32 TVBITS>
    static u32 toUTiny(const UPoint & v) {
      u32 x = toMag<TVBITS>(v.GetX());
      u32 y = toMag<TVBITS>(v.GetY());
      return (x<<TVBITS)|y;
    }

    template <u32 TVBITS>
    static SPoint toSPoint(const u32 bits) {
      const u32 MASK = (1<<TVBITS)-1;

      s32 x = fromSignMag<TVBITS>((bits>>TVBITS)&MASK);
      s32 y = fromSignMag<TVBITS>(bits&MASK);
      return SPoint(x,y);
    }

    template <u32 TVBITS>
    static UPoint toUPoint(const u32 bits) {
      const u32 MASK = (1<<TVBITS)-1;

      u32 x = fromMag<TVBITS>((bits>>TVBITS)&MASK);
      u32 y = fromMag<TVBITS>(bits&MASK);
      return UPoint(x,y);
    }

  public:
    const char* GetName() const { return "Bar"; }

    static Element_Bar THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    static const u32 TYPE_BITS = 8;
    static const u32 TYPE_MASK = (1<<TYPE_BITS)-1;
    static bool IsBarType(u32 type) {
      return type==TYPE;
    }

    static const u32 BITS_PER_DIM = 8;

    static const u32 STATE_SIZE_IDX = 0;
    static const u32 STATE_SIZE_LEN = 2 * BITS_PER_DIM;
    static const u32 STATE_POS_IDX = STATE_SIZE_IDX + STATE_SIZE_LEN;
    static const u32 STATE_POS_LEN = 2 * BITS_PER_DIM;
    static const u32 STATE_BITS = STATE_POS_IDX + STATE_POS_LEN;

    Element_Bar() { }

    SPoint GetMax(const T &atom) const {
      if (!IsBarType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      return MakeSigned(toUPoint<BITS_PER_DIM>(atom.GetStateField(STATE_SIZE_IDX,STATE_SIZE_LEN)));
    }

    SPoint GetPos(const T &atom) const {
      if (!IsBarType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      return MakeSigned(toUPoint<BITS_PER_DIM>(atom.GetStateField(STATE_POS_IDX,STATE_POS_LEN)));
    }

    bool FitsInRep(const SPoint & v) const {
      return v.BoundedBy(SPoint(0,0),SPoint((1<<BITS_PER_DIM)-1,(1<<BITS_PER_DIM)-1));
    }

    void SetSize(T &atom, const SPoint & v) const {
      if (!IsBarType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      if (!FitsInRep(v))
        FAIL(ILLEGAL_ARGUMENT);
      atom.SetStateField(STATE_SIZE_IDX,STATE_SIZE_LEN,toUTiny<BITS_PER_DIM>(MakeUnsigned(v)));
    }

    void SetPos(T &atom, const SPoint v) const {
      if (!IsBarType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      if (!FitsInRep(v))
        FAIL(ILLEGAL_ARGUMENT);
      atom.SetStateField(STATE_POS_IDX,STATE_POS_LEN,toUTiny<BITS_PER_DIM>(MakeUnsigned(v)));
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    T GetAtom(const SPoint & size, const SPoint & pos) const
    {
      T atom = GetDefaultAtom();
      SetSize(atom,size);
      SetPos(atom,pos);

      return atom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xfff00505;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff700202;
    }

    virtual u32 LocalPhysicsColor(const T & atom, u32 selector) const
    {
      switch (selector) {
      case 1: {
        SPoint barMax = GetMax(atom);
        SPoint myPos = GetPos(atom);
        return ColorMap_SEQ6_PuRd::THE_INSTANCE.
          GetInterpolatedColor(myPos.GetEuclideanLength(),
                               0,barMax.GetEuclideanLength(),
                               0xffff0000);
      }
      default:
        return Element<CC>::PhysicsColor();
      }
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      T self = window.GetCenterAtom();
      const u32 selfType = self.GetType();

      if (!IsBarType(selfType))
        FAIL(ILLEGAL_STATE);

      SPoint barMax = GetMax(self);

      SPoint myPos = GetPos(self);
      //      myPos.Print(stderr);

      const MDist<R> md = MDist<R>::get();

      SPoint anInconsistent;
      u32 inconsistentCount = 0;
      T unmakeGuy;

      u32 consistentCount = 0;

      SPoint toMake;
      T makeGuy;
      u32 makeCount = 0;

      SPoint toEat;
      u32 eatCount = 0;

      // Scan event window outside self
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(R); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        // First question: A 'bar grid' site?
        bool onGrid = (sp.GetX()&1)==0 && (sp.GetY()&1)==0;

        if (onGrid) {
          // Next question: Site within the bounds of the bar?

          const SPoint theirBarPos = sp+myPos;
          bool inBar = theirBarPos.BoundedBy(SPoint(0,0),barMax);

          if (inBar) {
            // Next question: Site empty?

            const T other = window.GetRelativeAtom(sp);
            const u32 otherType = other.GetType();

            bool isEmpty = otherType == Element_Empty<CC>::TYPE;

            if (isEmpty) {

              if (random.OneIn(++makeCount)) {
                toMake = sp;
                makeGuy = self; // Start with us
                SetPos(makeGuy,theirBarPos);  // Update position
                //window.SetRelativeAtom(sp,them);  // Store
              }

            } else {

              // Next question: Are they also a bar?
              if (IsBarType(otherType)) {

                // Next question: Are they consistent with us?

                SPoint otherBarMax = GetMax(other);
                SPoint otherPos = GetPos(other);

                SPoint otherPosMapped = otherPos-sp;
                if (otherBarMax==barMax && otherPosMapped == myPos) {
                  ++consistentCount;
                } else if (random.OneIn(++inconsistentCount)) {
                  anInconsistent = sp;
                  // Inconsistent Bars decay to Res
                  unmakeGuy = Element_Res<CC>::THE_INSTANCE.GetDefaultAtom();
                }
              }

            }
          }
        } else {
          // This is a non-grid site.  We'd kind of like it to be Res,
          // and if not that, empty, but we'd really like it not to
          // have Bar's in it

            const T other = window.GetRelativeAtom(sp);
            const u32 otherType = other.GetType();

            bool isRes = otherType == Element_Res<CC>::TYPE;
            if (isRes) {
              ++consistentCount;
              if (random.OneIn(++eatCount)) {
                toEat = sp;
              }
            }
            else {

              bool isEmpty = otherType == Element_Empty<CC>::TYPE;

              if (isEmpty) ++consistentCount;
              else {
                bool isBar = otherType == Element_Bar<CC>::TYPE;
                if (isBar) {
                  if (random.OneIn(++inconsistentCount)) {
                    anInconsistent = sp;
                    unmakeGuy = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
                  }
                }
              }
            }
        }
      }

      // Scan finished.  Let's decide what to do.

      // First question: Are we inconsistent with anybody?
      if (inconsistentCount > 0) {
        // Next question: Are we much more consistent than inconsistent?
        if (consistentCount > 3*inconsistentCount) {
          // Yes.  Punish selected loser
          window.SetRelativeAtom(anInconsistent, unmakeGuy);
        } else if (inconsistentCount > 3*consistentCount) {
          // If we're way inconsistent, let's res out and let them have us
          window.SetCenterAtom(Element_Res<CC>::THE_INSTANCE.GetDefaultAtom());
        }
      } else {
        // No inconsistencies.  Do we have something to make, and eat?
        if (makeCount > 0 && eatCount > 0) {
          window.SetRelativeAtom(toMake, makeGuy);
          window.SetRelativeAtom(toEat, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        }
      }

    }

  };

  template <class CC>
  Element_Bar<CC> Element_Bar<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_BAR_H */
