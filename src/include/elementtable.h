#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "bitfield.h"
#include "eventwindow.h"
#include "itype.h"
#include "p1atom.h"

typedef enum
{
  ELEMENT_NOTHING = 0x0,
  ELEMENT_DREG    = 0x1,
  ELEMENT_RES     = 0x2
}ElementType;

template <class T,u32 R>
class EventWindow;

template <class T,u32 R>
class ElementTable
{
private:
  /*
   * This is used to get the "state" field
   * out of an atom.
   */

  typedef u32 (* StateFunction )(T* atom);
  StateFunction m_statefunc;

  typedef void (* BehaviorFunction )(EventWindow<T,R>&, StateFunction f);
  BehaviorFunction m_funcmap[0xff];

  static void NothingBehavior(EventWindow<T,R>& w, StateFunction f);

  static void DRegBehavior(EventWindow<T,R>& w, StateFunction f);
  
public:
  ElementTable(u32 (*stateFunc)(T* atom));

  ~ElementTable() { }

  void Execute(EventWindow<T,R>& window)
  { m_funcmap[m_statefunc(&(window.GetCenterAtom()))](window, m_statefunc); }

  void FillAtom(T* atom, ElementType type);

  
};

#include "elementtable.tcc"

#endif /*ELEMENETTABLE_H*/
