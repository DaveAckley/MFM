#ifndef ELEMENT_PAIR_H   /* -*- C++ -*- */
#define ELEMENT_PAIR_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Res.h"
#include "itype.h"
#include "FXP.h"
#include "ByteSink.h"

namespace MFM
{

  /**
    A pair-forming class.

    Class:
    (C1) Name is Element_Pair
    (C2) Type is 0x2b2
    (C3) Role is u1
      (C3.1) Role 0 is Unemployed
      (C3.2) Role 1 is Paired
        (C3.2.1) Paired state is { side, a u1; loc, a V2D<2> }

    Role: Unemployed

    Behavior:

    - Search for otherLoc, a location containing other, the nearest
      unemployed Element_Pair

    if otherLoc == none, then { assert(OddsOf(19,20)); diffuse }
    else {
      self.role = Paired
      self.role.Paired.side = 1
      self.role.Paired.loc = otherLoc
      other.role = Paired
      other.role.Paired.side = 0
      other.role.Paired.loc = -otherLoc
      Place(self,(0,0))
      Place(other,otherLoc)
    }
    Done

    Role: Employed

    Element_Pair other = [self.role.Paired.loc] --- or FAIL
    assert(other.role == Paired)
    assert(other.role.Paired.side == 1-self.role.Paired.side)
    assert(other.role.Paired.loc == -otherLoc)

    Search for emptyLoc, an empty location less than distance 3
    from both self and other

    if (emptyLoc

    Behavior:

    - Search for otherLoc, a location containing other, the nearest
      unemployed Element_Pair

    if otherLoc == none, then { if OneIn(20) then destroy self else diffuse }
    else {
      self.role = Paired
      self.role.Paired.side = 1
      self.role.Paired.loc = otherLoc
      other.role = Paired
      other.role.Paired.side = 0
      other.role.Paired.loc = -otherLoc
      Place(self,(0,0))
      Place(other,otherLoc)
    }
    Done



      (1.1) If there exists i such that self.center info[i].center:

        Do (R1), then

        (1.1.1) Length(info[i].center) > 1:  --- We're not at center
          (1.1.1.1) There is an adjacent empty e closer than self to center
            Do (R5), (R6), then Done
          (1.1.1.2) --- No closer empty
            Do (R2) then Done
        (1.1.2) MinDist to ball <= 1
          (1.1.2.1) There is an adjacent empty e farther from ball
                    but no farther than self from center
            Do (R5), (R6), then Done


 :=> Stand

   */


  template <class T, u32 R>
  class Element_Pair : public Element<T,R>
  {

    class Vector {
      FXP16 x;
      FXP16 y;
    public:

      Vector() : x(0), y(0) { }
      Vector(FXP16 x, FXP16 y) : x(x), y(y) { }
      Vector(const SPoint p) : x(p.GetX()), y(p.GetY()) { }
      Vector(const UPoint p) : x(p.GetX()), y(p.GetY()) { }

      FXP16 GetX() const { return x; }

      FXP16 GetY() const { return y; }

      void Print(ByteSink & f) {
        f.Printf("(%06d,%06d)",x,y);
      }

      friend FXP16 GetDistSq(const Vector & v1, const Vector & v2)  {
        return (v1.x-v2.x)*(v1.x-v2.x)+(v1.y-v2.y)*(v1.y-v2.y);
      }

      friend Vector operator+(const Vector & v1, const Vector & v2) {
        return Vector(v1.x+v2.x,v1.y+v2.y);
      }
      friend Vector operator-(const Vector & v1) {
        return Vector(-v1.x,-v1.y);
      }
      friend Vector operator-(const Vector & v1, const Vector & v2) {
        return v1+-v2;
      }
      friend FXP16 operator*(const Vector & v1,const Vector v2) {
        return v1.x*v2.x+v1.y*v2.y;
      }
      friend Vector operator*(const FXP16 num, const Vector & v1) {
        return Vector(v1.x*num,v1.y*num);
      }
      friend Vector operator*(const Vector & v1,const FXP16 num) {
        return num*v1;
      }
      friend Vector operator/(const Vector & v1,const FXP16 num) {
        return 1.0/num*v1;
      }

      Vector & operator=(const Vector & v1) {
        this->x = v1.x;
        this->y = v1.y;
        return *this;
      }

      Vector & operator+=(const Vector & v1) {
        *this = *this + v1;
        return *this;
      }

      Vector & operator-=(const Vector & v1) {
        *this = *this - v1;
        return *this;
      }

      Vector & operator*=(const FXP16 num) {
        *this = *this * num;
        return *this;
      }

      Vector & operator/=(const FXP16 num) {
        *this = *this / num;
        return *this;
      }

    };

    template <u32 TVBITS>
    static u32 toSignMag(FXP16 value) {
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
    static FXP16 fromSignMag(const u32 value) {
      const u32 SIGN_BIT = 1<<(TVBITS-1);
      const u32 MASK = SIGN_BIT-1;
      FXP16 val = value&MASK;
      if (value & SIGN_BIT)
        val = -val;
      return val;
    }

    template <u32 TVBITS>
    static u32 toTinyVector(const Vector v) {
      u32 x = toSignMag<TVBITS>(v.x);
      u32 y = toSignMag<TVBITS>(v.y);
      return (x<<TVBITS)|y;
    }

    template <u32 TVBITS>
    static Vector toVector(const u32 bits) {
      const u32 MASK = (1<<TVBITS)-1;

      FXP16 x = fromSignMag<TVBITS>((bits>>TVBITS)&MASK);
      FXP16 y = fromSignMag<TVBITS>(bits&MASK);
      return Vector(x,y);
    }

  public:

    static const u32 TYPE = 0xbd;
    static const u32 TYPE_BITS = 8;
    static const u32 TYPE_MASK = (1<<TYPE_BITS)-1;
    static bool IsBoidType(u32 type) {
      return (type&TYPE_MASK)==TYPE;
    }

    static const u32 BITS_PER_DIM = 4;  // including 1 sign bit

    static const u32 STATE_HEADING_IDX = 0;
    static const u32 STATE_HEADING_LEN = 2 * BITS_PER_DIM;
    static const u32 STATE_BITS = STATE_HEADING_IDX + STATE_HEADING_LEN;

    Element_Pair() { }

    Vector GetHeading(const T &atom) const {
      if (!IsBoidType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      return toVector<BITS_PER_DIM>(atom.GetStateField(STATE_HEADING_IDX,STATE_HEADING_LEN));
    }

    void SetHeading(const T &atom, const Vector v) const {
      if (!IsBoidType(atom.GetType()))
        FAIL(ILLEGAL_STATE);
      atom.SetStateField(STATE_HEADING_IDX,STATE_HEADING_LEN,toTinyVector<BITS_PER_DIM>(v));
    }

    SPoint PickPossibleDestination(EventWindow<T,R>& window, const Vector & target, Random & random) const
    {
      SPoint dest;
      FXP16 totWgt = 0;

      const FXP16 MAX_DIST = R;
      const FXP16 MAX_DIST_WGT = R;
      const FXP16 WGT_PER_DIST = MAX_DIST_WGT / MAX_DIST;

      const MDist<R> md = MDist<R>::get();
      for (u32 idx = md.GetFirstIndex(0); idx < md.GetLastIndex(2); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        // Allow corners but not two steps away
        if (sp.GetX()>1 || sp.GetY()>1) continue;

        Vector spVec(sp);
        const T other = window.GetRelativeAtom(sp);

        // Compute weight of this choice:
        //
        // (1) Begin with dist from spVec to target: Closer is bigger,
        //     from MAX_DIST_WGT at dist 0 falling linearly to 0 at
        //     MAX_DIST
        //
        // (2) But check if the location is empty.  If not, cut that
        //     weight in half
        //
        // (3) Then add that weight to the total.
        //
        // (4) Then the dest is empty or the center, throw the
        //     weighted die and select if win

        FXP16 dist = GetDistSq(spVec,target);
        FXP16 weight = 0;
        if (dist < MAX_DIST)
          weight = MAX_DIST_WGT-dist*WGT_PER_DIST;
        if (other.GetType() != Element_Empty<T,R>::TYPE)
          weight /= 2;

        if (weight > 0) {
          totWgt += weight;
          if (random.OddsOf(weight,totWgt)) {
            dest = sp;
          }
        }
      }
      return dest;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      const double TARGET_SPACING = 1.5;
      Random & random = window.GetRandom();

      T self = window.GetCenterAtom();
      const u32 selfType = self.GetType();

      if (!IsBoidType(selfType))
        FAIL(ILLEGAL_STATE);

      //Vector myHead = GetHeading(self);
      //      myHead.Print(stderr);

      Vector avgHeading;
      Vector avgPosition;
      u32 countNgbr = 0;

      u32 minDist = R+1;

      u32 resCount = 0;
      SPoint resAt;

      const MDist<R> & md = MDist<R>::get();

      // Scan event window outside self
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(R); ++idx) {
        const SPoint sp = md.GetPoint(idx);
        const T other = window.GetRelativeAtom(sp);

        const u32 otherType = other.GetType();

        if (otherType == Element_Res<T,R>::TYPE) {
          if (random.OneIn(++resCount)) resAt = sp;
          continue;
        }

        if (otherType != selfType) continue;

        u32 dist = sp.GetManhattanLength();
        if (dist < minDist)
          minDist = dist;

        Vector theirHead = GetHeading(other);

        avgHeading += theirHead;
        avgPosition += Vector(sp);

        ++countNgbr;
      }

      if (countNgbr > 0 && countNgbr < 3 && resCount > 0) {
        // Whatever else we do, if we're not alone and not too crowded
        // and there's res, recruit it.  The scan is done though, so
        // we don't consider this guy in our heading calculations
	window.SetRelativeAtom(resAt, self);  // Clone bombs away
      }

      if (countNgbr == 0) {
        // We're alone, ono!  avgHeading and avgPosition are worthless!
        // With medium prob: Do random increment to heading
        // With lower prob: Suicide
        if (random.OneIn(3)) {
          window.SetCenterAtom(Element_Empty<T,R>::THE_INSTANCE.GetDefaultAtom());  // I can't stand it
        }

      } else if (countNgbr == 1 && minDist > TARGET_SPACING) {
        // Try to step closer to avgPosition (regardless of heading?)
        SPoint dest = PickPossibleDestination(window,avgPosition,random);

        window.SwapAtoms(SPoint(0,0),dest);
        return;  // No diffusion if swap

      } else {
        avgHeading /= countNgbr;
        avgPosition /= countNgbr;

        //        avgPosition.Print(stderr);
        //        fprintf(stderr," %d\n",countNgbr);
      }

      this->Diffuse(window);

    }

  };

}

#endif /* ELEMENT_PAIR_H */
