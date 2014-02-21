#ifndef ELEMENT_DREG_H
#define ELEMENT_DREG_H

#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"
#include "element_res.h"  /* For Element_Res::TYPE */

namespace MFM
{

#define DREG_RES_ODDS 20
#define DREG_DEL_ODDS 10
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 /*Deleting DREGs*/

  template <class T, u32 R>
  class Element_Dreg : public Element<T,R>
  {

  public:
    static Element_Dreg THE_INSTANCE;
    static const u32 TYPE = 0xdba;             // We are the death-birth-agent

    Element_Dreg() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff505050;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      Random & random = window.GetRandom();
      
      SPoint dir;
      ManhattanDir<R>::get().FillRandomSingleDir(dir);
      
      
      T atom = window.GetRelativeAtom(dir);
      u32 oldType = atom.GetType();
      
      if(oldType == ELEMENT_NOTHING)
      {
	if(random.OneIn(DREG_DRG_ODDS))
	{
	  atom = Element_Dreg<T,R>::THE_INSTANCE.GetDefaultAtom();
	}
	else if(random.OneIn(DREG_RES_ODDS))
	{
	  atom = Element_Res<T,R>::THE_INSTANCE.GetDefaultAtom();
	}
      }
      else if(oldType == Element_Dreg::TYPE)
      {
	if(random.OneIn(DREG_DDR_ODDS))
	{
	  atom = T();
	}
      }
      else if(random.OneIn(DREG_DEL_ODDS))
      {
	  atom = T();
      }
      
      if(atom.GetType() != oldType)
      {
	window.SetRelativeAtom(dir, atom);
      }

      this->Diffuse(window);
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Dreg<T,R> Element_Dreg<T,R>::THE_INSTANCE;

  /*
  template <class T, u32 R>
  void Element_Dreg<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Dreg<T,R>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_DREG_H */
