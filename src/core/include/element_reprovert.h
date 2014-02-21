#ifndef ELEMENT_REPROVERT_H       /* -*- C++ -*- */
#define ELEMENT_REPROVERT_H

#include "element.h"
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{


  template <class T, u32 R>
  class Element_Reprovert : public Element<T,R>
  {

  public:
    static const u32 STATE_VERTPOS_IDX = 0;
    static const u32 STATE_VERTPOS_LEN = 10;
    static const u32 STATE_BITS = STATE_VERTPOS_IDX+STATE_VERTPOS_LEN;

    u32 GetVertPos(const T &atom, u32 badType) const {
      if (!atom.IsType(Element<T,R>::GetType())) return badType;
      return atom.GetStateField(STATE_VERTPOS_IDX,STATE_VERTPOS_LEN);
    }

    bool SetVertPos(T &atom, u32 value) const {
      if (!atom.IsType(Element<T,R>::GetType())) return false;
      atom.SetStateField(STATE_VERTPOS_IDX,STATE_VERTPOS_LEN,value);
      return true;
    }


    void ReproduceVertically(EventWindow<T,R>& window) const;

  };

  template <class T, u32 R>
    void Element_Reprovert<T,R>::ReproduceVertically(EventWindow<T,R>& window) const
  {
    Random & random = window.GetRandom();
    
    s32 cval = GetVertPos(window.GetCenterAtom(),0);
    bool down = random.CreateBool();
    SPoint repPt(0, down ? R/2 : -(R/2));
    if(window.GetRelativeAtom(repPt).GetType() == ELEMENT_NOTHING)
    {
      window.SetRelativeAtom(repPt, this->GetDefaultAtom());
      SetVertPos(window.GetRelativeAtom(repPt),(u32) (cval + (down ? 1 : -1)));
    }
  }


}


#endif /* ELEMENT_REPROVERT_H */
