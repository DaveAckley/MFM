#include "assert.h"
#include "MDist_Test.h"

namespace MFM {

void MDist_Test::Test_MDistConversion()
{
  SPoint longPt(2, 2);
  SPoint shortPt(1, -1);

  u8 longBits  = MDist<4>::get().FromPoint(longPt, MANHATTAN_TABLE_LONG);
  u8 shortBits = MDist<4>::get().FromPoint(shortPt, MANHATTAN_TABLE_SHORT);

  SPoint out(0, 0);

  MDist<4>::get().FillFromBits(out, longBits, MANHATTAN_TABLE_LONG);

  assert(out.GetX() == 2);
  assert(out.GetY() == 2);

  MDist<4>::get().FillFromBits(out, shortBits, MANHATTAN_TABLE_SHORT);

  assert(out.GetX() == 1);
  assert(out.GetY() == -1);
}
} /* namespace MFM */
