#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "BitVector.h"
#include "EventWindow.h"
#include "Dirs.h"
#include "itype.h"
#include "Element.h"

namespace MFM {

  template <class CC>
  class EventWindow;

  template <class CC>
  class ElementTable
  {
    // Extract short type names
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { B = P::ELEMENT_TABLE_BITS};
    enum { R = P::EVENT_WINDOW_RADIUS};
    enum { ELEMENT_DATA_SLOTS = P::ELEMENT_DATA_SLOTS};

  public:
    static const u32 SIZE = 1u<<B;

    /**
     * Reinitialize this ElementTable to empty.
     */
    void Reinit() ;

    void Insert(const Element<CC> & theElement) ;

    u32 GetSize() const {
      return SIZE;
    }

    /**
     * Return -1 if elementType is not stored in this table, otherwise
     * return a number from 0.. GetSize()-1 representing the location
     * of this elementType in this table.
     */
    s32 GetIndex(u32 elementType) const ;

    ElementTable();

    ~ElementTable() { }

    const Element<CC> * Lookup(u32 elementType) ;

    void Execute(EventWindow<CC>& window)
    {
      u32 type = window.GetCenterAtom().GetType();
      if(type != ELEMENT_EMPTY) {
        const Element<CC> * elt = Lookup(type);
        if (elt == 0) FAIL(UNKNOWN_ELEMENT);
        elt->Behavior(window);
      }
    }

    bool RegisterElement(const Element<CC>& e)
    {
      Insert(e);
      return true;
    }

    /**
     * Allocate SLOTS u64's of element-specific data associated with
     * element E.
     *
     * Returns false if:
     *
     * - E is an unregistered element
     *
     * - E is a registered element but has already had a different
     *   number of element-specific data slots allocated for it
     *
     * - E is a registered element with no priori element-specific
     *   data allocation, but less than SLOTS of room for
     *   element-specific data remain in this ElementTable.
     *
     * Returns true otherwise, specifically, if:
     *
     * - E is a registered element that already has precisely SLOTS of
     *   element-specific data allocated for it, or
     *
     * - E is a registered element that previously had no
     *   element-specific data but now has SLOTS of element-specific
     *   data allocated for it, as a result of this call.
     *
     * In the case of a true return, note that the resulting slots
     * have no particular values.
     */
    bool AllocateElementDataSlots(const Element<CC>& e, u32 slots) {
      u32 elementType = e.GetType();
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

    /**
     * Access the SLOTS u64's of element-specific data associated with
     * element E, if it exists.  Returns a pointer to the first (0th)
     * slot, or 0.
     *
     * Returns 0 if:
     *
     * - E is an unregistered element,
     *
     * - E is a registered element that has no element-specific data
     *   associated with it, or
     *
     * - E is a registered element that has more or less than SLOTS of
     *   element-specific data associated with it.
     */
    u64 * GetElementDataSlots(const Element<CC>& e, u32 slots) {
      u32 elementType = e.GetType();
      s32 index = GetIndex(elementType);
      if (index < 0) return 0;

      if (m_hash[index].m_elementDataLength == 0) return 0;
      if (m_hash[index].m_elementDataLength != slots) return 0;
      return & m_elementData[m_hash[index].m_elementDataStart];
    }

  private:

    u32 SlotFor(u32 elementType) const ;

    struct ElementEntry {
      void Clear() {
        m_element = 0;
        m_elementDataStart = 0;
        m_elementDataLength = 0;
      }
      const Element<CC>* m_element;
      u16 m_elementDataStart;
      u16 m_elementDataLength;
    } m_hash[SIZE];
    u32 m_hashSlotsInUse;

    u64 m_elementData[ELEMENT_DATA_SLOTS];
    u32 m_nextFreeElementDataIndex;


  };

} /* namespace MFM */

#include "ElementTable.tcc"

#endif /*ELEMENETTABLE_H*/
