#include "elementtype.h"
#include "eventwindow.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  typedef u32 (* StateFunction )(P1Atom* atom);


#define DREG_RES_ODDS 20
#define DREG_DEL_ODDS 10
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 /*Deleting DREGs*/

  template <u32 R>
  static void DRegBehavior(EventWindow<P1Atom,R>& window, StateFunction f)
  {
    Random & random = window.GetRandom();

    SPoint dir;
    ManhattanDir<R>::get().FillRandomSingleDir(dir);

    u32 state = f(&window.GetRelativeAtom(dir));

    ElementType newType = (ElementType)-1;

    if(state == ELEMENT_NOTHING)
    {
      if(random.OneIn(DREG_DRG_ODDS))
	{
	  newType = ELEMENT_DREG;
	}
      else if(random.OneIn(DREG_RES_ODDS))
	{
	  newType = ELEMENT_RES;
	}
    }
    else if(state == ELEMENT_DREG)
    {
      if(random.OneIn(DREG_DDR_ODDS))
	{
	  newType = ELEMENT_NOTHING;
	}
    }
    else if(random.OneIn(DREG_DEL_ODDS))
    {
      newType = ELEMENT_NOTHING;
    }

    if(newType >= 0)
    {
      window.SetRelativeAtom(dir, P1Atom(newType));
    }
  }

  /*
  bool element_dreg_registered = 
    ElementTable<P1Atom,4>::get().RegisterElement(ELEMENT_DREG, &DRegBehavior);
  */

}
