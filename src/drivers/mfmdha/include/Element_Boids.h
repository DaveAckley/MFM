#ifndef ELEMENT_BOIDS_H   /* -*- C++ -*- */
#define ELEMENT_BOIDS_H

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
    A clump-forming class.  Goals:

    - Survive and operate in a DReg environment, recruiting Res to
      establish and maintain the clump.

    - Approximately but effectively regulate clump size even though
      the size will be larger than an event window radius.

    - Maintain 'clump identity' by a shared belief about the position
      of the center or centroid of the clump.  Nearby clumps will not
      coalesce in calm conditions due to their separate identities.

    - An ability to maintain and (later) move the clump position.

    Class:
    (C1) Name is Ball
    (C2) Type is 0xba11
    (C3) State is BallState

    Sets:
    (S1) BallState is { center, when }, with
      (S1.1) center, a TinyVec<3> --- heading to believed center
      (S1.2) when, a u3 --- events since last two+ point confirmation on center.
                        --- max value for 'when' means no confidence in 'center'
    (S2) BallInfo is { center, count, distance, nearus, adjemp }, with
      (S2.1) center, a Point --- location of claimed Ball center
      (S2.2) count, a u8     --- number of votes for that center
      (S2.3) distance, a u8  --- distance to nearest-us voter for this center
      (S2.4) nearus, a Point  --- location of nearest-us voter for this center
      (S2.5) adjemp, a Point  --- location of Moore neighborhood empty nearest this center

    (S3) Centers is Array[limit 3] of BallInfo
    (S4) Empties is Array[limit 7] of Point --- Moore neighborhood empties

    Actors:
    (A1) self, a Ball
    (A2) info, a Centers
    (A3) empties, an Empties
    (A4) i, an Index into info
    (A5) e, an Index into empties

    Self check:

    - None

    Perception:

    - Collect potential clump centers + exact match confirmation
      counts, for, say, up to three centers.  Include self.  Also for
      each potential clump, collect distance to nearest of that clump.

    - Also collect Moore neighborhood live empties

    Reactions:

    (R1) self.when = 0

    (R2) Place(self), then
    (R3) Done

    (R4) self.when = 0, then
    (R5) self.center += empties[e]-self, then
    (R6) self <=> empties[e], then
    (R7) Done


    Thought:

    --- (1) If I'm part of a confirmed ball stay near center
    --- (2)

    (1) If info.length > 0: --- At least one confirmed center

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


  template <class CC>
  class Element_Boids : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

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
        f.Printf("(%06x,%06x)",x.asInt(),y.asInt());
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

    Element_Boids() { }

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

    SPoint PickPossibleDestination(EventWindow<CC>& window, const Vector & target, Random & random) const
    {
      SPoint dest;
      FXP16 totWgt = 0;

      const FXP16 MAX_DIST = R;
      const FXP16 MAX_DIST_WGT = R;
      const FXP16 WGT_PER_DIST = MAX_DIST_WGT / MAX_DIST;

      const MDist<R> & md = MDist<R>::get();
      for (u32 idx = md.GetFirstIndex(0); idx <= md.GetLastIndex(2); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        // Allow corners but not two steps away
        if (sp.GetMaximumLength() > 1) continue;

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
        if (other.GetType() != Element_Empty<CC>::TYPE)
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

    virtual void Behavior(EventWindow<CC>& window) const
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

        if (otherType == Element_Res<CC>::TYPE) {
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
          window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());  // I can't stand it
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

#endif /* ELEMENT_BOIDS_H */
