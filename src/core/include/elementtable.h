#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "bitfield.h"
#include "eventwindow.h"
#include "eucliddir.h"
#include "itype.h"
#include "p1atom.h"

#define ELEMENT_COUNT 7

typedef enum
{
  ELEMENT_NOTHING  = 0x0,
  ELEMENT_DREG     = 0x1,
  ELEMENT_RES      = 0x2,
  ELEMENT_SORTER   = 0x3,
  ELEMENT_EMITTER  = 0x4,
  ELEMENT_CONSUMER = 0x5,
  ELEMENT_DATA     = 0x6
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

  /* the atomCounts argument can be NULL on any call to these functions. */
  typedef void (* BehaviorFunction )
  (EventWindow<T,R>&, StateFunction f, s32* atomCounts);

  BehaviorFunction m_funcmap[0xff];

  /* Fills pt with the coordinates of a randomly selected          */
  /* Atom with the specified type. Returns false if there is none. */
  static bool FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
				      ElementType type, StateFunction f, 
				      EuclidDir corner);

  static void NothingBehavior(EventWindow<T,R>& w, StateFunction f, s32* atomCounts);

  static void DRegBehavior(EventWindow<T,R>& w, StateFunction f, s32* atomCounts);

  static void SorterBehavior(EventWindow<T,R>& w, StateFunction f, s32* atomCounts);

  static void EmitterBehavior(EventWindow<T,R>& w, StateFunction f, s32* atomCounts);

  static void ConsumerBehavior(EventWindow<T,R>& w, StateFunction f, s32* atomCounts);

  static void ReproduceVertically(EventWindow<T,R>& w, 
				  StateFunction f,
				  ElementType type, s32* atomCounts);
  
public:
  ElementTable(u32 (*stateFunc)(T* atom));

  ~ElementTable() { }

  void Execute(EventWindow<T,R>& window, s32* atomCounts)
  { m_funcmap[m_statefunc(&(window.GetCenterAtom()))](window, m_statefunc, atomCounts); }

  void FillAtom(T* atom, ElementType type);

  bool Diffusable(ElementType type);
  
};

#include "elementtable.tcc"

#endif /*ELEMENETTABLE_H*/
