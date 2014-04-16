#ifndef ELEMENT_DATA_H       /* -*- C++ -*- */
#define ELEMENT_DATA_H

#include "Element.h"
#include "ColorMap.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

#define DATA_MAXVAL 1000000
#define DATA_MINVAL 1

namespace MFM
{

  template <class CC>
  class Element_Data : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;

  public:
    const char* GetName() const { return "Data"; }

    static Element_Data THE_INSTANCE;
    static const u32 TYPE = 0xdada;
    static const u32 STATE_DATA_IDX = 0;
    static const u32 STATE_DATA_LEN = 32;
    static const u32 STATE_BITS = STATE_DATA_IDX+STATE_DATA_LEN;

    u32 GetDatum(const T &atom, u32 badType) const {
      if (!Atom<CC>::IsType(atom,TYPE)) return badType;
      return atom.GetStateField(STATE_DATA_IDX,STATE_DATA_LEN);
    }

    bool SetDatum(T &atom, u32 value) const {
      if (!Atom<CC>::IsType(atom,TYPE)) return false;
      atom.SetStateField(STATE_DATA_IDX,STATE_DATA_LEN,value);
      return true;
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff0000ff;
    }

    virtual u32 LocalPhysicsColor(const T & atom, u32 selector) const
    {
      switch (selector) {
      case 1:
        return ColorMap_SEQ6_PuBuGn::THE_INSTANCE.
          GetInterpolatedColor(GetDatum(atom,0),DATA_MINVAL,DATA_MAXVAL,0xffff0000);
      default:
        return DefaultPhysicsColor();
      }
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {

      u32 val = GetDatum(window.GetCenterAtom(),-1);
      if (val < DATA_MINVAL || val > DATA_MAXVAL)
        FAIL(ILLEGAL_STATE);

      this->Diffuse(window);
    }

    static void Needed();
  };

  template <class CC>
  Element_Data<CC> Element_Data<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_DATA_H */
