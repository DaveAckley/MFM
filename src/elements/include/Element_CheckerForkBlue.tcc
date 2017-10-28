/* -*- C++ -*- */
#include "Element_CheckerForkRed.h"

namespace MFM
{
  template <class EC>
  void Element_CheckerForkBlue<EC>::Behavior(EventWindow<EC>& window) const
  {
    SPoint pt;
    int i = 0;

    for(Dir d = Dirs::NORTH; i < 4; d = Dirs::CWDir(Dirs::CWDir(d)), i++)
    {
      Dirs::FillDir(pt, d, false);
      if(window.IsLiveSiteSym(pt))
      {
        window.SetRelativeAtomSym(pt, Element_CheckerForkRed<EC>::THE_INSTANCE.GetDefaultAtom());
      }
    }
  }
}
