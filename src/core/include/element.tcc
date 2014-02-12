#include "fail.h" /* -*- C++ -*- */

namespace MFM
{

  template <class T, u32 R>
  void ReproduceVertically(EventWindow<T,R>& window, u32 (* f)(T* atom),
			   ElementType type)
  {
    Random & random = window.GetRandom();
    
    u32 cval = window.GetCenterAtom().ReadLowerBits();
    bool down = random.CreateBool();
    SPoint repPt(0, down ? R/2 : -(R/2));
    if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
    {
      window.SetRelativeAtom(repPt, T(type));
      window.GetRelativeAtom(repPt).WriteLowerBits(cval + (down ? 1 : -1));
    }
  }
}
