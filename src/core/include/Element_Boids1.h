#ifndef ELEMENT_BOIDS1_H
#define ELEMENT_BOIDS1_H

#include "Element.h"       /* -*- C++ -*- */
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Res.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{


  template <class T, u32 R>
  class Element_Boids1 : public Element<T,R>
  {
    class Vector {
      double x;
      double y;
    public:
      Vector(double x, double y) : x(x), y(y) { }
      Vector() : x(0), y(0) { }

      double GetX() const { return x; }

      double GetY() const { return y; }

      void Print(FILE * f) {
        fprintf(f,"(%06f,%06f)",x,y);
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
      friend double  operator*(const Vector & v1,const Vector v2) {
        return v1.x*v2.x+v1.y*v2.y;
      }
      friend Vector operator*(const double num, const Vector & v1) {
        return Vector(v1.x*num,v1.y*num);
      }
      friend Vector operator*(const Vector & v1,const double num) {
        return num*v1;
      }
      friend Vector operator/(const Vector & v1,const double num) {
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

      Vector & operator*=(const double num) {
        *this = *this * num;
        return *this;
      }

      Vector & operator/=(const double num) {
        *this = *this / num;
        return *this;
      }

    };

    template <u32 TVBITS> 
    static u32 toSignMag(double value) {
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
    static double fromSignMag(const u32 value) {
      const u32 SIGN_BIT = 1<<(TVBITS-1);
      const u32 MASK = SIGN_BIT-1;
      double val = value&MASK;
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

      double x = fromSignMag<TVBITS>((bits>>TVBITS)&MASK);
      double y = fromSignMag<TVBITS>(bits&MASK);
      return Vector(x,y);
    }

  public:
    const char* GetName() const { return "Boid"; }

    static Element_Boids1 THE_INSTANCE;
    static const u32 TYPE = 0xbd1;

    static const u32 BITS_PER_DIM = 4;  // including 1 sign bit

    static const u32 STATE_HEADING_IDX = 0;
    static const u32 STATE_HEADING_LEN = 2*BITS_PER_DIM;
    static const u32 STATE_BITS =  STATE_HEADING_IDX+STATE_HEADING_LEN;

    Element_Boids1() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff00f0f0;
    }
    
    Vector GetHeading(const T &atom) const {
      if (!atom.IsType(TYPE)) 
        FAIL(ILLEGAL_STATE);
      return toVector<BITS_PER_DIM>(atom.GetStateField(STATE_HEADING_IDX,STATE_HEADING_LEN));
    }

    void SetHeading(const T &atom, const Vector v) const {
      if (!atom.IsType(TYPE)) 
        FAIL(ILLEGAL_STATE);
      atom.SetStateField(STATE_HEADING_IDX,STATE_HEADING_LEN,toTinyVector<BITS_PER_DIM>(v));
    }

    virtual void Behavior(EventWindow<T,R>& window) const 
    {
      const double TARGET_SPACING = 1.5;
      Random & random = window.GetRandom();

      T self = window.GetCenterAtom();
      if (self.GetType() != TYPE)
        FAIL(ILLEGAL_STATE);

      //Vector myHead = GetHeading(self);
      //      myHead.Print(stderr);

      Vector avgHeading;
      Vector avgPosition;
      u32 countNgbr = 0;

      u32 minDist = R+1;

      u32 resCount = 0;
      SPoint resAt;

      const MDist<R> md = MDist<R>::get();
      for (u32 idx = md.GetFirstIndex(1); idx < md.GetLastIndex(R); ++idx) {
        const SPoint sp = md.GetPoint(idx);
        const T other = window.GetRelativeAtom(sp);

        const u32 otherType = other.GetType();

        if (otherType == Element_Res<T,R>::TYPE) {
          if (random.OneIn(++resCount)) resAt = sp;
          continue;
        }

        if (otherType != TYPE) continue;
        
        u32 dist = sp.GetManhattanDistance();
        if (dist < minDist)
          minDist = dist;

        Vector theirHead = GetHeading(other);

        avgHeading += theirHead;
        avgPosition += Vector(sp.GetX(),sp.GetY());

        ++countNgbr;
      }

      if (countNgbr != 0 && resCount > 0) {
        // Whatever else we do, if we're not alone and there's res,
        // recruit it
	window.SetRelativeAtom(resAt, self);  // Clone bombs away
      }

      if (countNgbr == 0) {
        // We're alone, ono!
        // With medium prob: Do random increment to heading
        // With lower prob: Suicide

      } else if (countNgbr == 1 && minDist > TARGET_SPACING) {
        // Try to step closer to avgPosition (regardless of heading?)

      } else {
        avgHeading /= countNgbr;
        avgPosition /= countNgbr;
        //        avgPosition.Print(stderr);
        //        fprintf(stderr," %d\n",countNgbr);
      }

      this->Diffuse(window);

    }

  };

  template <class T, u32 R>
  Element_Boids1<T,R> Element_Boids1<T,R>::THE_INSTANCE;

}

#endif /* ELEMENT_BOIDS1_H */
