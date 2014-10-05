/* -*- C++ -*- */
#include "Element_CheckerForkBlue.h"

namespace MFM
{
  template <class CC>
  void Element_CheckerForkRed<CC>::Behavior(EventWindow<CC>& window) const
  {
    SPoint pt;
    int i = 0;
    for(Dir d = Dirs::NORTH; i < 4; d = Dirs::CWDir(Dirs::CWDir(d)), i++)
    {
      Dirs::FillDir(pt, d);
      if(window.IsLiveSite(pt))
      {
        window.SetRelativeAtom(pt, Element_CheckerForkBlue<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }
  }
}
