/* -*- C++ -*- */
#include "Element_CheckerForkRed.h"

namespace MFM
{
  template <class CC>
  void Element_CheckerForkBlue<CC>::Behavior(EventWindow<CC>& window) const
  {
    SPoint pt;
    int i = 0;
    for(Dir d = Dirs::NORTH; i < 4; d = Dirs::CWDir(Dirs::CWDir(d)), i++)
    {
      Dirs::FillDir(pt, d);
      if(window.IsLiveSite(pt))
      {
        window.SetRelativeAtom(pt, Element_CheckerForkRed<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }
  }
}
