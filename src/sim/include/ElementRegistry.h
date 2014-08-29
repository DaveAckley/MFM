/*                                              -*- mode:C++ -*-
  ElementRegistry.h A simulation-wide registry of available elements by UUID
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
  \file ElementRegistry.h A simulation-wide registry of available elements by UUID
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENTREGISTRY_H
#define ELEMENTREGISTRY_H

#include "itype.h"
#include "Element.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {

  /**
   * The ElementRegistry holds information about all Elements known to
   * a simulation, whether or not they are actually loaded into the
   * running image, and whether or not they (currently) have been
   * 'Needed' into a grid and thereby assigned a type number.  It is
   * meant for use only during initialization and is therefore dog
   * slow.  It also maintains a search list of directories within
   * which it will search for dynamically loadable elements, on
   * demand.
   */
  template <class CC>
  class ElementRegistry
  {
  public:
    enum {
      TABLE_SIZE = 100,
      MAX_PATHS = 20,
      MAX_PATH_LEN = 256
    };

  private:

    // Extract short type names
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    typedef OverflowableCharBufferByteSink<MAX_PATH_LEN> PathString ;

  public:

    ElementRegistry();

    ~ElementRegistry() { }

    void Init();

    bool IsRegistered(const UUID & uuid) const;

    bool IsLoaded(const UUID & uuid) const;

    bool Load(const UUID & uuid) ;

    Element<CC> * Lookup(const UUID & uuid) const;

    Element<CC> * LookupCompatible(const UUID & uuid) const;

    /**
     * Add a path to the search path, if it is not already there.
     * Fails with OUT_OF_ROOM if no more paths can be added.
     */
    void AddPath(const char * path) ;

    /**
     * Store a dynamically-loadable element in the registry keyed by
     * its UUID, which is (allegedly) loadable somewhere along the
     * search path
     */
    bool RegisterUUID(const UUID & uuid) ;

    /**
     * Store an element in the registry keyed by its uuid
     */
    bool RegisterElement(Element<CC>& e) ;

    /**
     * Gets the number of currently registered Elements inside this
     * ElementRegistry.
     */
    u32 GetEntryCount() const
    {
      return m_registeredElementsCount;
    }

    /**
     * Gets the UUID of the Element which was loaded in a particular
     * index.
     *
     * @param entryIdx The index at which this UUID was loaded. For
     *                 instance, the UUID at index \c 0 was loaded first.
     *
     * @returns The UUID of the element loaded at this partuclar index.
     */
    const UUID& GetEntryUUID(u32 entryIdx) const
    {
      if (entryIdx >= GetEntryCount())
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      return m_registeredElements[entryIdx].m_uuid;
    }

    /**
     * Gets a pointer to the specific Element which was loaded in a
     * particular index.
     *
     * @param entryIdx The index at which this Element was loaded. For
     *                 instance, the Element at index \c 0 was loaded first.
     *
     * @returns The Element of the element loaded at this partuclar index.
     */
    const Element<CC>* GetEntryElement(u32 entryIdx) const
    {
      if (entryIdx >= GetEntryCount())
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      return m_registeredElements[entryIdx].m_element;
    }


  private:
    struct ElementEntry {
      UUID m_uuid;                   //< Set in all cases
      Element<CC>* m_element;  //< Set if element is loaded
      s32 m_pathIndex;               //< Set if the element was found in this pathentry

      ElementEntry() : m_element(0), m_pathIndex(-1) { }
    } m_registeredElements[TABLE_SIZE];
    u32 m_registeredElementsCount;

    const ElementEntry * FindMatching(const UUID & uuid) const {
      for (u32 i = 0; i < m_registeredElementsCount; ++i) {
        if (m_registeredElements[i].m_uuid == uuid)
          return &m_registeredElements[i];
      }
      return 0;
    }

    ElementEntry * FindMatching(const UUID & uuid) {
      for (u32 i = 0; i < m_registeredElementsCount; ++i) {
        if (m_registeredElements[i].m_uuid == uuid)
          return &m_registeredElements[i];
      }
      return 0;
    }

    s32 FindCompatibleIndex(const UUID & uuid, s32 lastIndex) const {
      if (lastIndex < -1)
        FAIL(ILLEGAL_ARGUMENT);
      for (s32 i = lastIndex + 1; i < (s32) m_registeredElementsCount; ++i) {
        if (m_registeredElements[i].m_uuid.Compatible(uuid))
          return i;
      }
      return -1;
    }

    PathString m_searchPaths[MAX_PATHS];
    u32 m_searchPathsCount;

  };

} /* namespace MFM */

#include "ElementRegistry.tcc"

#endif /*ELEMENTREGISTRY_H*/
