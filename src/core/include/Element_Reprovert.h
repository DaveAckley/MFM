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
    static const u32 STATE_BELOW_IDX = 0;
    static const u32 STATE_BELOW_LEN = 9;
    static const u32 STATE_ABOVE_IDX = STATE_BELOW_IDX+STATE_BELOW_LEN;
    static const u32 STATE_ABOVE_LEN = 9;
    static const u32 STATE_GAP_IDX = STATE_ABOVE_IDX+STATE_ABOVE_LEN;
    static const u32 STATE_GAP_LEN = 2;
    static const u32 STATE_BITS = STATE_GAP_IDX+STATE_GAP_LEN;

    static const u32 MAX_BELOW = (1<<STATE_BELOW_LEN)-1;
    static const u32 MAX_ABOVE = (1<<STATE_ABOVE_LEN)-1;

    u32 GetBelow(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return badType;
      return atom.GetStateField(STATE_BELOW_IDX,STATE_BELOW_LEN);
    }

    bool SetBelow(T &atom, u32 value) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return false;
      atom.SetStateField(STATE_BELOW_IDX,STATE_BELOW_LEN,value);
      return true;
    }

    u32 GetAbove(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return badType;
      return atom.GetStateField(STATE_ABOVE_IDX,STATE_ABOVE_LEN);
    }

    bool SetAbove(T &atom, u32 value) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return false;
      atom.SetStateField(STATE_ABOVE_IDX,STATE_ABOVE_LEN,value);
      return true;
    }

    u32 GetGap(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,Element<CC>::GetType())) return badType;
      return atom.GetStateField(STATE_GAP_IDX,STATE_GAP_LEN)+1;
    }

    bool SetGap(T &atom, u32 value) const {
      if (value==0 || !Atom<CC>::IsType(atom,Element<CC>::GetType())) return false;
      atom.SetStateField(STATE_GAP_IDX,STATE_GAP_LEN,value-1);
      return true;
    }


    void ReproduceVertically(EventWindow<CC>& window) const;

  };

  template <class CC>
  void Element_Reprovert<CC>::ReproduceVertically(EventWindow<CC>& window) const
  {
    Random & random = window.GetRandom();
    T self = window.GetCenterAtom();
    s32 gap = GetGap(self,0);

    u32 selfType = self.GetType();
    s32 belowSelf = GetBelow(self,0);
    s32 aboveSelf = GetAbove(self,0);
    bool down = random.CreateBool();
    SPoint repPt(0, down ? gap : -gap);
    T other = window.GetRelativeAtom(repPt);
    u32 otherType = other.GetType();

    bool weChanged = false;

    if (otherType == ELEMENT_EMPTY) {
      T newAtom = self;

      // New guys are assumed maximally extreme in their direction
      // from us, so if they're stuck in the edge of the universe
      // getting no events, we'll believe their numbers.  When the
      // empty is just a dreg glitch, hopefully they'll listen to
      // their neighbors faster than their neighbors listen to them.
      SetBelow(newAtom, down || belowSelf==0 ? 0 : belowSelf + 1);
      SetAbove(newAtom, !down || aboveSelf==0 ? 0 : aboveSelf + 1);
      window.SetRelativeAtom(repPt, newAtom);

    } else if (otherType == selfType) {
      s32 belowThem = GetBelow(other,0);
      s32 aboveThem = GetAbove(other,0);

      if (down && belowThem + 1 != belowSelf) {
        SetBelow(self, belowThem + 1);
        weChanged = true;
      }

      if (!down && aboveThem + 1 != aboveSelf) {
        SetAbove(self, aboveThem + 1);
        weChanged = true;
      }

    } else {

      // Something not empty and not one of us is there.  We will
      // slowly decide we are the last guy in that direction.

      if (down && belowSelf > 0 && random.OneIn(10)) {
        --belowSelf;
        SetBelow(self, belowSelf);
        weChanged = true;
      } else if (!down && aboveSelf > 0 && random.OneIn(10)) {
        --aboveSelf;
        SetAbove(self, aboveSelf);
        weChanged = true;
      }
    }

    if (weChanged)
      window.SetCenterAtom(self);
  }


}


#endif /* ELEMENT_REPROVERT_H */
