#ifndef ELEMENT_DATA_H       /* -*- C++ -*- */
#define ELEMENT_DATA_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

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

    Element_Data() {}
    
    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff0000ff;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {

      u32 val = GetDatum(window.GetCenterAtom(),-1);
      if (val < 0 || val > 100)
        FAIL(ILLEGAL_STATE);

      this->Diffuse(window);
    }

    static void Needed();    
  };

  template <class CC>
  Element_Data<CC> Element_Data<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_DATA_H */
