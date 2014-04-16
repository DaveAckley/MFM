#ifndef ELEMENT_REPROVERT_H       /* -*- C++ -*- */
#define ELEMENT_REPROVERT_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{


  template <class CC>
  class Element_Reprovert : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    static const u32 STATE_VERTPOS_IDX = 0;
    static const u32 STATE_VERTPOS_LEN = 10;
    static const u32 STATE_BITS = STATE_VERTPOS_IDX+STATE_VERTPOS_LEN;

    u32 GetVertPos(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return badType;
      return atom.GetStateField(STATE_VERTPOS_IDX,STATE_VERTPOS_LEN);
    }

    bool SetVertPos(T &atom, u32 value) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return false;
      atom.SetStateField(STATE_VERTPOS_IDX,STATE_VERTPOS_LEN,value);
      return true;
    }


    void ReproduceVertically(EventWindow<CC>& window, u32 id) const;

  };

  template <class CC>
  void Element_Reprovert<CC>::ReproduceVertically(EventWindow<CC>& window, u32 id) const
  {
    Random & random = window.GetRandom();
    
    s32 cval = GetVertPos(window.GetCenterAtom(),0);
    bool down = random.CreateBool();
    SPoint repPt(0, down ? R/2 : -(R/2));
    if(window.GetRelativeAtom(repPt).GetType() == ELEMENT_EMPTY)
    {
      T newAtom = this->GetDefaultAtom();
      u32 newval = (u32) (cval + (down ? 1 : -1));
      SetVertPos(newAtom, newval);
      window.SetRelativeAtom(repPt, newAtom);
    }
  }


}


#endif /* ELEMENT_REPROVERT_H */
