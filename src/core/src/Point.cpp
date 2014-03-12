#include "Point.h"

namespace MFM {

UPoint makeUnsigned(const SPoint & spoint) 
{
  if (spoint.GetX() < 0 || spoint.GetY() < 0)
    FAIL(ILLEGAL_ARGUMENT);
  return UPoint((u32) spoint.GetX(), (u32) spoint.GetY());
}

SPoint makeSigned(const UPoint & upoint)
{
  if (upoint.GetX() > (u32) S32_MAX || upoint.GetY() > (u32) S32_MAX)
    FAIL(ILLEGAL_ARGUMENT);
  return SPoint((s32) upoint.GetX(), (s32) upoint.GetY());
}
} /* namespace MFM */

