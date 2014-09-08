#include "Point.h"

namespace MFM
{
  UPoint MakeUnsigned(const SPoint & spoint)
  {
    if (!CanMakeUnsigned(spoint))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return UPoint((u32) spoint.GetX(), (u32) spoint.GetY());
  }

  SPoint MakeSigned(const UPoint & upoint)
  {
    if (!CanMakeSigned(upoint))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return SPoint((s32) upoint.GetX(), (s32) upoint.GetY());
  }

} /* namespace MFM */
