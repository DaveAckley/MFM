#ifndef ELEMENT_DATA_H       /* -*- C++ -*- */
#define ELEMENT_DATA_H

#include "element.h"
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Data : public Element<T,R>
  {
  public:
    static Element_Data THE_INSTANCE;
    static const u32 TYPE = 0xdada;
    static const u32 STATE_DATA_IDX = 0;
    static const u32 STATE_DATA_LEN = 32;
    static const u32 STATE_BITS = STATE_DATA_IDX+STATE_DATA_LEN;

    u32 GetDatum(const T &atom, u32 badType) const {
      if (!atom.IsType(TYPE)) return badType;
      return atom.GetStateField(STATE_DATA_IDX,STATE_DATA_LEN);
    }

    bool SetDatum(T &atom, u32 value) const {
      if (!atom.IsType(TYPE)) return false;
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

    virtual void Behavior(EventWindow<T,R>& window) const
    {

      u32 val = GetDatum(window.GetCenterAtom(),-1);
      if (val < 0 || val > 100)
        FAIL(ILLEGAL_STATE);

      this->Diffuse(window);
    }

    static void Needed();    
  };

  template <class T, u32 R>
  Element_Data<T,R> Element_Data<T,R>::THE_INSTANCE;

}

#endif /* ELEMENT_DATA_H */
