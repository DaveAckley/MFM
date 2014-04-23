#include <stdlib.h> /* -*- C++ -*- */
#include "Dirs.h"
#include "MDist.h"

namespace MFM {

  template <class C>
  s32 ElementTable<C>::GetIndex(u32 elementType) const
  {
    u32 slot = SlotFor(elementType);
    if (m_hash[slot].m_element == 0) return -1;
    return (s32) slot;
  }

  template <class C>
  u32 ElementTable<C>::SlotFor(u32 elementType) const
  {
    u32 collide = 0;
    u32 slot = elementType;
    while (true) {
      slot %= SIZE;
      const Element<C> * elt = m_hash[slot].m_element;
      if (elt==0 || elt->GetType() == elementType)
        return slot;   // Empty or match: This is the slot for you
      ++collide;
      slot = elementType+(collide*(1+collide))/2;
    }
  }

  template <class C>
  void ElementTable<C>::Insert(const Element<C> & theElement)
  {
    u32 type = theElement.GetType();
    u32 slotFor = SlotFor(type);

    if (m_hash[slotFor].m_element != 0) {

      if (m_hash[slotFor].m_element != &theElement)
        FAIL(DUPLICATE_ELEMENT_TYPE);
      else return;

    } else {
      if (++m_hashSlotsInUse > SIZE/2)
        FAIL(TOO_MANY_ELEMENT_TYPES);  // XXX standardize on OUT_OF_ROOM ?
      m_hash[slotFor].m_element = &theElement;

    }
  }

  template <class C>
  const Element<C> * ElementTable<C>::Lookup(u32 elementType)
  {
    return m_hash[SlotFor(elementType)].m_element;
  }

  template <class C>
  ElementTable<C>::ElementTable()
  {
    Reinit();
  }

  template <class C>
  void ElementTable<C>::Reinit()
  {
    m_hashSlotsInUse = 0;
    for (u32 i = 0; i < SIZE; ++i)
      m_hash[i].Clear();
    m_nextFreeElementDataIndex = 0;
  }

} /* namespace MFM */
