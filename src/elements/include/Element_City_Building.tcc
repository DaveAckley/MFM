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
  bool Element_City_Building<CC>::DoNBSidewalkCase(EventWindow<CC>& window) const
  {
    MDist<R>& md = MDist<R>::get();
    SPoint sidewalk;

    if(window.FindRandomLocationOfType(GetSidewalkType(), 1, sidewalk))
    {
      Dir swDir = Dirs::FromOffset(swDir);

      SPoint adjs[2];

      Dirs::FillDir(adjs[0], Dirs::CWDir(swDir));
      Dirs::FillDir(adjs[1], Dirs::CCWDir(swDir));

      for(u32 i = 0; i < 2; i++)
      {
        const T& at = window.GetRelativeAtom(adjs[i]);
        if(at.GetType() != TYPE())
        {
          T copyMe = window.GetCenterAtom();
          u32 idx = LargetsVisibleIndex(window) + 1;

          if(idx <= GetMaxArea(copyMe))
          {
            SetAreaIndex(copyMe, idx);
            window.SetRelativeAtom(adjs[i], copyMe);
          }
        }
      }
    }
    return false;
  }

  template <class CC>
  bool Element_City_Building<CC>::DoNBPerpGrowthCase(EventWindow<CC>& window) const
  {
    return false;
  }

  template <class CC>
  bool Element_City_Building<CC>::DoNBCornerGrowthCase(EventWindow<CC>& window) const
  {
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
