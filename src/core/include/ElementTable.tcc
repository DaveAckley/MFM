/* -*- C++ -*- */
#include <stdlib.h>
#include "Dirs.h"
#include "MDist.h"
#include "Element.h"

namespace MFM {

  template <class EC>
  s32 ElementTable<EC>::GetIndex(u32 elementType) const
  {
    u32 slot = SlotFor(elementType);
    if (m_hash[slot].m_element == 0) return -1;
    return (s32) slot;
  }

  template <class EC>
  u32 ElementTable<EC>::SlotFor(u32 elementType) const
  {
    u32 collide = 0;
    u32 slot = elementType;
    while (true)
    {
      slot %= SIZE;
      const Element<EC> * elt = m_hash[slot].m_element;
      if (elt==0 || elt->GetType() == elementType)
        return slot;   // Empty or match: This is the slot for you
      ++collide;
      slot = elementType+(collide*(1+collide))/2;
    }
  }

  template <class EC>
  void ElementTable<EC>::Insert(const Element<EC> & theElement)
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

  template <class EC>
  const Element<EC> * ElementTable<EC>::ReplaceEmptyElement(const Element<EC> & newEmptyElement)
  {
    enum { ATOM_EMPTY_TYPE = EC::ATOM_CONFIG::ATOM_EMPTY_TYPE };

    MFM_API_ASSERT_ARG(newEmptyElement.GetType() == ATOM_EMPTY_TYPE); // New guy must think it's the empty element

    u32 eslot = SlotFor(ATOM_EMPTY_TYPE);
    const Element<EC> * old = m_hash[eslot].m_element;

    MFM_API_ASSERT_STATE(old && old->GetType() == ATOM_EMPTY_TYPE);   // Must have old guy that also thinks it's the empty element

    //    MFM_ASSERT_API_STATE(old != &newEmptyElement);   // Must not be the same guy (can we require this?  loses idempotency)

    m_hash[eslot].m_element = &newEmptyElement; // And so the deed is done; have mercy on our souls.

    return old;
  }


  template <class EC>
  const Element<EC> * ElementTable<EC>::Lookup(u32 elementType) const
  {
    return m_hash[SlotFor(elementType)].m_element;
  }

  template <class EC>
  const Element<EC> * ElementTable<EC>::Lookup(const u8 * symbol) const
  {
    MFM_API_ASSERT_NONNULL(symbol);

    const Element<EC> * found = 0;
    for (u32 i = 0; i < SIZE; ++i) 
    {
      if (m_hash[i].m_element == 0) continue;
      if (!strcmp(m_hash[i].m_element->GetAtomicSymbol(),(const char *) symbol)) 
      {
        if (found) return 0;  // multiple hits
        found = m_hash[i].m_element;
      }
    }

    return found;
  }

  template <class EC>
  bool ElementTable<EC>::RegisterElement(const Element<EC>& e)
  {
    Insert(e);
    return true;
  }

  template <class EC>
  ElementTable<EC>::ElementTable()
  {
    Reinit();
  }

#if 0
  template <class EC>
  bool ElementTable<EC>::AllocateElementDataSlots(const Element<EC>& e, u32 slots)
  {
    u32 elementType = e.GetType();
    return AllocateElementDataSlotsFromType(elementType, slots);
  }

  template <class EC>
  bool ElementTable<EC>::AllocateElementDataSlotsFromType(const u32 elementType, u32 slots)
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

  template <class EC>
  u64 * ElementTable<EC>::GetElementDataSlots(const Element<EC>& e, const u32 slots)
  {
    u32 elementType = e.GetType();
    return GetElementDataSlotsFromType(elementType, slots);
  }

  template <class EC>
  u64 * ElementTable<EC>::GetElementDataSlotsFromType(const u32 elementType, const u32 slots)
  {
    s32 index = GetIndex(elementType);
    if (index < 0) return 0;

    if (m_hash[index].m_elementDataLength == 0) return 0;
    if (m_hash[index].m_elementDataLength != slots) return 0;
    return & m_elementData[m_hash[index].m_elementDataStart];
  }

  template <class EC>
  u64 * ElementTable<EC>::GetDataAndRegister(const u32 elementType, u32 slots)
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

  template <class EC>
  u64 * ElementTable<EC>::GetDataIfRegistered(const u32 elementType, u32 slots)
  {
    return GetElementDataSlotsFromType(elementType, slots);
  }
#endif

  template <class EC>
  void ElementTable<EC>::Reinit()
  {
    m_hashSlotsInUse = 0;
    for (u32 i = 0; i < SIZE; ++i)
      m_hash[i].Clear();
    //XXX    m_nextFreeElementDataIndex = 0;
  }

} /* namespace MFM */
