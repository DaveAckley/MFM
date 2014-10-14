/* -*- C++ -*- */

#include "Element_City_Sidewalk.h"

namespace MFM
{
  template <class CC>
  u32 Element_City_Building<CC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  bool DoNBSidewalkCase(EventWindow<CC>& window) const
  {
    MDist<R>& md = MDist<R>::get();

    for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
    {
      SPoint pt = md.GetPoint(i);

      if(window.GetRelativeAtom(pt).GetType() == GetSidewalkType())
      {
        Dir swDir = Dirs::FromOffset(pt);
        SPoint adjPt;

        Dirs::FillDir(adj, Dirs::CWDir(swDir));

        if(window.GetRelativeAtom(pt).GetType() ==
           Element_Empty<CC>::THE_INSTANCE.GetType())
        {
          u32 largestIdx = LargestVisibleIndex(window);

          T newMe  = window.GetCenterAtom();

          SetAreaIndex(newMe, largestIdx + 1);

          window.SetRelativeAtom(pt, newMe);

          return true;
        }
      }
    }
    return false;
  }

  template <class CC>
  void Element_City_Building<CC>::SpawnNextBuilding(EventWindow<CC>& window) const
  {
    if(!DoNBSidewalkCase(window))
    {
      if(!DoNBPerpGrowthCase(window))
      {
        DoNBCornerGrowthCase(window);
      }
    }
  }
}
