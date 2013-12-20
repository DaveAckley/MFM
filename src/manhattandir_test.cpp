#include "assert.h"
#include "manhattandir_test.hpp"

void ManhattanDirTest::Test_manhattandirConversion()
{
  ManhattanDir::AllocTables();

  Point<int> longPt(2, 2);
  Point<int> shortPt(1, -1);
  
  u8 longBits  = ManhattanDir::FromPoint(&longPt, false);
  u8 shortBits = ManhattanDir::FromPoint(&shortPt, true);

  Point<int> out(0, 0);

  ManhattanDir::FillFromBits(&out, longBits, false);

  assert(out.GetX() == 2);
  assert(out.GetY() == 2);

  ManhattanDir::FillFromBits(&out, shortBits, true);

  assert(out.GetX() == 1);
  assert(out.GetY() == -1);
}
