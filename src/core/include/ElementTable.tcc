/* -*- C++ -*- */
#include <stdlib.h>
#include "Dirs.h"
#include "MDist.h"
#include "Element.h"
#include "Element_Empty.h"

namespace MFM {

  template <class CC>
  s32 ElementTable<CC>::GetIndex(u32 elementType) const
  {
    u32 slot = SlotFor(elementType);
    if (m_hash[slot].m_element == 0) return -1;
    return (s32) slot;
  }

  template <class CC>
  u32 ElementTable<CC>::SlotFor(u32 elementType) const
  {
    u32 collide = 0;
    u32 slot = elementType;
    while (true)
    {
      slot %= SIZE;
      const Element<CC> * elt = m_hash[slot].m_element;
      if (elt==0 || elt->GetType() == elementType)
        return slot;   // Empty or match: This is the slot for you
      ++collide;
      slot = elementType+(collide*(1+collide))/2;
    }
  }

  template <class CC>
  void ElementTable<CC>::Insert(const Element<CC> & theElement)
  {
    u32 type = theElement.GetType();
    u32 slotFor = SlotFor(type);

    if (m_hash[slotFor].m_element != 0) {

      if (m_hash[slotFor].m_element != &theElement)
        FAIL(DUPLICATE_ENTRY);
      else return;

    } else {
      if (++m_hashSlotsInUse > SIZE/2)
        FAIL(OUT_OF_ROOM);
      m_hash[slotFor].m_element = &theElement;

    }
  }

  template <class CC>
  const Element<CC> * ElementTable<CC>::Lookup(u32 elementType) const
  {
    return m_hash[SlotFor(elementType)].m_element;
  }

  template <class CC>
  void ElementTable<CC>::Execute(EventWindow<CC>& window)
  {
    T atom = window.GetCenterAtom();
    if (!atom.IsSane())
    {
      if (atom.HasBeenRepaired())
      {
        window.SetCenterAtom(atom);
      }
      else
      {
        FAIL(INCONSISTENT_ATOM);
      }
    }
    u32 type = atom.GetType();
    if(type != Element_Empty<CC>::THE_INSTANCE.GetType())
    {
      const Element<CC> * elt = Lookup(type);
      if (elt == 0) FAIL(UNKNOWN_ELEMENT);
      elt->Behavior(window);
    }
  }

  template <class CC>
  bool ElementTable<CC>::RegisterElement(const Element<CC>& e)
  {
    Insert(e);
    return true;
  }

  template <class CC>
  ElementTable<CC>::ElementTable()
  {
    Reinit();
  }

  template <class CC>
  bool ElementTable<CC>::AllocateElementDataSlots(const Element<CC>& e, u32 slots)
  {
    u32 elementType = e.GetType();
    return AllocateElementDataSlotsFromType(elementType, slots);
  }

  template <class CC>
  bool ElementTable<CC>::AllocateElementDataSlotsFromType(const u32 elementType, u32 slots)
  {
    s32 index = GetIndex(elementType);
    if (index < 0) return false;

    if (m_hash[index].m_elementDataLength != 0) {
      if (m_hash[index].m_elementDataLength != slots)
        return false;
    } else {
      if (m_nextFreeElementDataIndex+slots > ELEMENT_DATA_SLOTS)
        return false;

      m_hash[index].m_elementDataLength = slots;
      m_hash[index].m_elementDataStart = m_nextFreeElementDataIndex;
      m_nextFreeElementDataIndex += slots;
    }

    return true;
  }

  template <class CC>
  u64 * ElementTable<CC>::GetElementDataSlots(const Element<CC>& e, const u32 slots)
  {
    u32 elementType = e.GetType();
    return GetElementDataSlotsFromType(elementType, slots);
  }

  template <class CC>
  u64 * ElementTable<CC>::GetElementDataSlotsFromType(const u32 elementType, const u32 slots)
  {
    s32 index = GetIndex(elementType);
    if (index < 0) return 0;

    if (m_hash[index].m_elementDataLength == 0) return 0;
    if (m_hash[index].m_elementDataLength != slots) return 0;
    return & m_elementData[m_hash[index].m_elementDataStart];
  }

  template <class CC>
  u64 * ElementTable<CC>::GetDataAndRegister(const u32 elementType, u32 slots)
  {
    // Yes, (this use of) the EDS API is pretty awkward.

    // See if already registered
    u64 * datap = GetElementDataSlotsFromType(elementType, slots);

    if (!datap)
    {

      // Not yet registered.  If this fails, you probably need to up
      // the EDS in your ParamConfig.
      if (!AllocateElementDataSlotsFromType(elementType, slots))
        FAIL(ILLEGAL_STATE);

      // Now this must succeed
      datap = GetElementDataSlotsFromType(elementType, slots);
      if (!datap)
        FAIL(ILLEGAL_STATE);

      // Init newly registered slots
      for (u32 i = 0; i < slots; ++i)
        datap[i] = 0;
    }
    return datap;
  }

  template <class CC>
  u64 * ElementTable<CC>::GetDataIfRegistered(const u32 elementType, u32 slots)
  {
    return GetElementDataSlotsFromType(elementType, slots);
  }

  template <class CC>
  void ElementTable<CC>::Reinit()
  {
    m_hashSlotsInUse = 0;
    for (u32 i = 0; i < SIZE; ++i)
      m_hash[i].Clear();
    m_nextFreeElementDataIndex = 0;
  }

} /* namespace MFM */
