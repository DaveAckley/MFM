#ifndef VECTOR_H   /* -*- C++ -*- */
#define VECTOR_H

#include "FXP.h"
#include "Point.h"

namespace MFM {
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
}
#endif /* VECTOR_H */
