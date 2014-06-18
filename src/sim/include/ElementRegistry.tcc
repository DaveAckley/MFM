#include <stdlib.h> /* -*- C++ -*- */

namespace MFM {
  template <class CC>
  bool ElementRegistry<CC>::RegisterElement(const Element<CC>& e)
  {
    if (IsRegistered(e.GetUUID()))
      return false;
    if (m_registeredElementsCount >= TABLE_SIZE)
      FAIL(OUT_OF_ROOM);
    ElementEntry & ee =  m_registeredElements[m_registeredElementsCount++];
    ee.m_uuid = e.GetUUID();
    ee.m_element = &e;
    return true;
  }

  template <class CC>
  bool ElementRegistry<CC>::RegisterUUID(const UUID & uuid)
  {
    if (IsRegistered(uuid))
      return false;
    if (m_registeredElementsCount >= TABLE_SIZE)
      FAIL(OUT_OF_ROOM);
    ElementEntry & ee =  m_registeredElements[m_registeredElementsCount++];
    ee.m_uuid = uuid;
    ee.m_element = 0;
    return true;
  }

  template <class CC>
  const Element<CC> * ElementRegistry<CC>::Lookup(const UUID & uuid) const
  {
    const ElementEntry * ee = FindMatching(uuid);
    if (ee) return ee->m_element;
    return 0;
  }

  template <class CC>
  const Element<CC> * ElementRegistry<CC>::LookupCompatible(const UUID & uuid) const
  {
    // Try exact match first
    const Element<CC> * ret = Lookup(uuid);
    if (ret) return ret;

    s32 index = FindCompatibleIndex(uuid, -1);
    if (index < 0)
      return 0;

    return m_registeredElements[index].m_element;
  }

  template <class CC>
  bool ElementRegistry<CC>::IsRegistered(const UUID & uuid) const
  {
    for (u32 i = 0; i < m_registeredElementsCount; ++i) {
      if (m_registeredElements[i].m_uuid==uuid)
        return true;
    }
    return false;
  }

  template <class CC>
  bool ElementRegistry<CC>::IsLoaded(const UUID & uuid) const
  {
    for (u32 i = 0; i < m_registeredElementsCount; ++i) {
      const ElementEntry & ee = m_registeredElements[i];
      if (ee.m_uuid==uuid)
        return ee.m_element != 0;
    }
    return false;
  }

  template <class CC>
  void ElementRegistry<CC>::AddPath(const char * path)
  {
    if (!path)
      FAIL(NULL_POINTER);
    for (u32 i = 0; i < m_searchPathsCount; ++i) {
      if (m_searchPaths[i].Equals(path))
        return;
    }
    if (m_searchPathsCount >= MAX_PATHS)
      FAIL(OUT_OF_ROOM);
    m_searchPaths[m_searchPathsCount] = path;
    ++m_searchPathsCount;
  }
  template <class CC>
  ElementRegistry<CC>::ElementRegistry()
    : m_registeredElementsCount(0), m_searchPathsCount(0)
  {

  }

} /* namespace MFM */
