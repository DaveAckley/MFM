#include "Point.h"

namespace MFM
{
  UPoint MakeUnsigned(const SPoint & spoint)
  {
    MFM_API_ASSERT_ARG(CanMakeUnsigned(spoint));
    return UPoint((u32) spoint.GetX(), (u32) spoint.GetY());
  }

  SPoint MakeSigned(const UPoint & upoint)
  {
    MFM_API_ASSERT_ARG(CanMakeSigned(upoint));
    return SPoint((s32) upoint.GetX(), (s32) upoint.GetY());
  }

} /* namespace MFM */
