#ifndef ELEMENTTABLE_HPP
#define ELEMENTTABLE_HPP

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

template <class T>
class EventWindow;

template <class T>
class ElementTable
{
private:
  void (* m_funcmap[0xff])(EventWindow<T>&);

  /* Can't typedef this. Gross! */

  /*
   * This is used to get the "state" field
   * out of an atom.
   */

  u32 (*m_statefunc)(T* atom);

  static void NothingBehavior(EventWindow<T>& w);

  static void DRegBehavior(EventWindow<T>& w);
  
public:
  ElementTable(u32 (*stateFunc)(T* atom));

  ~ElementTable() { }

  void Execute(EventWindow<T>& window)
  { m_funcmap[m_statefunc(&window.GetCenterAtom())](window); }

  void FillAtom(T* atom, ElementType type);

  
};

#include "elementtable.tcc"

#endif /*ELEMENETTABLE_HPP*/
