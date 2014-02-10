#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "bitfield.h"
#include "eventwindow.h"
#include "eucliddir.h"
#include "itype.h"
#include "p1atom.h"
#include "elementtype.h"

namespace MFM {

#define ELEMENT_COUNT 7


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
  (EventWindow<T,R>&, StateFunction f);

  BehaviorFunction m_funcmap[0xff];

  static void FlipSEPointToCorner(Point<s32>& pt, EuclidDir corner);

  /* Fills pt with the coordinates of a randomly selected          */
  /* Atom with the specified type. Returns false if there is none. */
  static bool FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
				      ElementType type, StateFunction f, 
				      EuclidDir corner);


  static void FillSubwindowIndices(s32* indices, EventWindow<T,R>& window,StateFunction f,
				   ElementType type, EuclidDir corner);

  static u32 FoundIndicesCount(s32* indices);

  static void NothingBehavior(EventWindow<T,R>& w, StateFunction f);

  static void DRegBehavior(EventWindow<T,R>& w, StateFunction f);

  static void SorterBehavior(EventWindow<T,R>& w, StateFunction f);

  static void EmitterBehavior(EventWindow<T,R>& w, StateFunction f);

  static void ConsumerBehavior(EventWindow<T,R>& w, StateFunction f);

  static void ReproduceVertically(EventWindow<T,R>& w, 
				  StateFunction f,
				  ElementType type);
  
public:
  ElementTable();

  static ElementTable<T,R> & get();

  ~ElementTable() { }

  void Execute(EventWindow<T,R>& window)
  { m_funcmap[m_statefunc(&(window.GetCenterAtom()))](window, m_statefunc); }

  void SetStateFunction(StateFunction f);

  void FillAtom(T* atom, ElementType type);

  bool Diffusable(ElementType type);

  /* Registers a new BehaviorFunction to this ElementTable. Returns */
  /* true if an element definition was not overwritten.             */
  bool RegisterElement(ElementType type, BehaviorFunction function)
  {
    bool alreadyRegistered = m_funcmap[(u32)type] == NULL;
    m_funcmap[(u32)type] = function;
    return alreadyRegistered;
  }
  
};

} /* namespace MFM */

#include "elementtable.tcc"

#endif /*ELEMENETTABLE_H*/
