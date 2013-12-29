#include "assert.h"
#include "point_test.hpp"
#include "point.h"

void PointTest::Test_pointAdd()
{
  Point<int> iPointA(20, 30);
  Point<int> iPointB(30, 20);

  iPointA.Add(iPointB);

  assert(iPointA.GetX() == 50);
  assert(iPointA.GetY() == 50);

  Point<float> fPointA(30.0f, 50.0f);
  Point<float> fPointB(50.0f, 30.0f);

  fPointA.Add(fPointB);

  assert(fPointA.GetX() == 30.0f + 50.0f);
}

void PointTest::Test_pointMultiply()
{
  Point<int> iPoint(20, 100);
  Point<double> dPoint(100.0, 20.25);

  iPoint.Multiply(30);

  dPoint.Multiply(2.0);

  assert(iPoint.GetX() == 600);
  assert(iPoint.GetY() == 3000);

  assert(dPoint.GetX() == 100.0 * 2.0);
  assert(dPoint.GetY() == 20.25 * 2.0);
}
