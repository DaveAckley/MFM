#include <boost/test/minimal.hpp>
#include "point_test.hpp"
#include "point.h"

void PointTest::Test_pointAdd()
{
  Point<int> iPointA(20, 30);
  Point<int> iPointB(30, 20);

  iPointA.Add(&iPointB);

  BOOST_CHECK(iPointA.GetX() == 50);
  BOOST_CHECK(iPointA.GetY() == 50);

  Point<float> fPointA(30.0f, 50.0f);
  Point<float> fPointB(50.0f, 30.0f);

  fPointA.Add(&fPointB);

  BOOST_CHECK(fPointA.GetX() == 30.0f + 50.0f);
}
