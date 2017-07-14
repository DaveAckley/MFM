/*                                              -*- mode:C++ -*-
  ElementTable.h Registry of elements
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file ElementTable.h Registry of elements
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENTTABLE_H
#define ELEMENTTABLE_H

#include "BitVector.h"
#include "Dirs.h"
#include "itype.h"

namespace MFM
{

  template <class EC> class Element; // FORWARD
  template <class EC> class EventWindow; // FORWARD

  typedef u32 ElementType;

  template <class EC>
  class ElementTable
  {
    // Extract short type names
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { B = AC::ATOM_TYPE_BITS};
    enum { R = EC::EVENT_WINDOW_RADIUS};
    //XXX currently deprecated    enum { ELEMENT_DATA_SLOTS = P::ELEMENT_DATA_SLOTS};

  public:
    // -3 to avoid 2**k and 2**k-1 sizes; they seem to beat against type assignments
    static const u32 SIZE = (1u<<(B/2)) - 3; // ~250

    /**
     * Reinitialize this ElementTable to empty.
     */
    void Reinit() ;

    /**
     * Registers an Element into this ElementTable, allowing Elements
     * of a specified type to exist and act inside a Tile .
     *
     * @param theElement The Element to insert into this ElementTable .
     */
    void Insert(const Element<EC> & theElement) ;

    /**
     * Replace an existing Element that represents empty space with an
     * alternate Element that performs the same function.  The table
     * must have an old element with ATOM_EMPTY_TYPE already in it.
     * The new element must also already have been given type
     * ATOM_EMPTY_TYPE.  This method is meant for use by compiled ulam
     * setup code to replace Element_Empty (an archaic Element<EC>),
     * with Ue_10105Empty10 (a shiny modern UlamElement<EC>).
     *
     * @param newEmptyType The replacement Element to use as the empty
     * element in this ElementTable .
     *
     * @return the old, now replaced, empty element
     *
     * @fails ILLEGAL_ARGUMENT if the new element is not already
     * assigned ATOM_EMPTY_TYPE
     *
     * @fails ILLEGAL_STATE if there is no existing element in the
     * expected location, or if the element located there is not type
     * ATOM_EMPTY_TYPE
     */
    const Element<EC> * ReplaceEmptyElement(const Element<EC> & newEmptyElement) ;

    /**
     * Gets the capacity of this ElementTable, in Elements that may be
     * registered.
     */
    u32 GetSize() const
    {
      return SIZE;
    }

    /**
     * Return -1 if elementType is not stored in this table, otherwise
     * return a number from 0.. GetSize()-1 representing the location
     * of this elementType in this table.
     */

    /**
     * Gets the index of this ElementTable where a particular Element
     * (described by a specified type) resides.
     *
     * @param elementType The type of the Element of which to look up
     *                    an index for.
     *
     * @returns The index of this ElementTable where the Element of
     *          type \c elementType resides, or -1 if \c elementType
     *          is not found in the table.
     */
    s32 GetIndex(u32 elementType) const ;

    /**
     * Constructs and calls \c Reinit() on a new new ElementTable.
     */
    ElementTable();

    /**
     * Deconstructs this ElementTable.
     */
    ~ElementTable() { }

    /**
     * Gets a pointer to an immutable Element which is stored in this
     * ElementTable by providing a type.
     *
     * @param elementType the type of the Element which will be found
     *                    in this Table.
     *
     * @returns A pointer to an immutable Element which is stored in
     *          this table. If an Element with this type is not found
     *          in this ElementTable, will return NULL .
     */
    const Element<EC> * Lookup(u32 elementType) const;

    /**
     * Gets a pointer to an immutable Element which is stored in this
     * ElementTable by providing its atomic symbol
     *
     * @param symbol the atomic symbol of this Element which will be found
     *                    in this Table.
     *
     * @returns A pointer to an immutable Element which is stored in
     *          this table. If an Element with this atomic is not
     *          found -- OR IF MULTIPLE Elements WITH THIS SYMBOL ARE
     *          FOUND -- in this ElementTable, will return NULL .
     */
    const Element<EC> * Lookup(const u8 * symbol) const;

#if 0 /* Now handled in eventwindow */
    /**
     * Executes the behavior method of the Element in the center of a
     * specified EventWindow. This method finds the central Element by
     * the type of the Atom located there, then executes its behavior.
     *
     * @param window The EventWindow to execute an event upon.
     */
    void Execute(EventWindow<EC>& window) ;
#endif

    /**
     * Inserts an Element into this ElementTable.
     *
     * @param e The Element to insert into this ElementTable.
     *
     * @returns \c true .
     */
    bool RegisterElement(const Element<EC>& e) ;

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
    bool AllocateElementDataSlots(const Element<EC>& e, u32 slots) ;

    bool AllocateElementDataSlotsFromType(const u32 elementType, u32 slots) ;

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
    u64 * GetElementDataSlots(const Element<EC>& e, const u32 slots) ;

    u64 * GetElementDataSlotsFromType(const u32 elementType, const u32 slots) ;

    u64 * GetDataAndRegister(const u32 elementType, u32 slots) ;

    u64 * GetDataIfRegistered(const u32 elementType, u32 slots) ;

  private:

    /**
     * Acting as a hashing function, finds a slot inside this
     * ElementTable table for an Element with a given type.
     *
     * @param elementType The type of an Element to find a slot in
     * this ElementTable for.
     *
     * @returns The index of the slot of the internal table which
     *          belongs to an Element of \c elementType type.
     */
    u32 SlotFor(u32 elementType) const ;

    struct ElementEntry {
      void Clear() {
        m_element = 0;
        m_elementDataStart = 0;
        m_elementDataLength = 0;
      }
      const Element<EC>* m_element;
      u16 m_elementDataStart;
      u16 m_elementDataLength;
    } m_hash[SIZE];
    u32 m_hashSlotsInUse;

  };

} /* namespace MFM */

#include "ElementTable.tcc"

#endif /*ELEMENTTABLE_H*/
