/* -*- C++ -*- */
#include "Element_CheckerForkBlue.h"

namespace MFM
{
  template <class EC>
  void Element_CheckerForkRed<EC>::Behavior(EventWindow<EC>& window) const
  {
    SPoint pt;
    int i = 0;

    for(Dir d = Dirs::NORTH; i < 4; d = Dirs::CWDir(Dirs::CWDir(d)), i++)
    {
      Dirs::FillDir(pt, d);
      if(window.IsLiveSiteSym(pt))
      {
        window.SetRelativeAtomSym(pt, Element_CheckerForkBlue<EC>::THE_INSTANCE.GetDefaultAtom());
      }
    }
  }
}
