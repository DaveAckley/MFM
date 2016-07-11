/* -*- C++ -*- */
#include <stdlib.h> /* For getenv */
#include <errno.h>  /* For errno */
#include <string.h> /* For strerror */
#include <dirent.h> /* For opendir */
#include "ElementLibraryLoader.h"
#include "Utils.h"
#include "Fail.h"

namespace MFM
{

  template <class EC>
  void ElementRegistry<EC>::Init(UlamClassRegistry<EC> & ucr)
  {
    LOG.Debug("Loading ElementLibraries from %d file(s)...", m_libraryPathsCount);
    s32 elements = LoadLibraries(ucr);
    if (elements < 0)
    {
      FAIL(ILLEGAL_STATE);
    }
    LOG.Debug("Got %d elements", elements);
  }

  template <class EC>
  u32 ElementRegistry<EC>::GetRegisteredElementCount() const
  {
    return m_registeredElementsCount;
  }

  template <class EC>
  Element<EC> * ElementRegistry<EC>::GetRegisteredElement(u32 index)
  {
    if (index >= m_registeredElementsCount)
      FAIL(ILLEGAL_ARGUMENT);
    ElementEntry & ee =  m_registeredElements[index];
    return ee.m_element;
  }

  template <class EC>
  bool ElementRegistry<EC>::RegisterElement(Element<EC>& e)
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

  template <class EC>
  bool ElementRegistry<EC>::RegisterUUID(const UUID & uuid)
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

  template <class EC>
  Element<EC> * ElementRegistry<EC>::Lookup(const UUID & uuid) const
  {
    const ElementEntry * ee = FindMatching(uuid);
    if (ee) return ee->m_element;
    return 0;
  }

  template <class EC>
  Element<EC> * ElementRegistry<EC>::LookupCompatible(const UUID & uuid) const
  {
    // Try exact match first
    Element<EC> * ret = Lookup(uuid);
    if (ret) return ret;

    s32 index = FindCompatibleIndex(uuid, -1);
    if (index < 0)
      return 0;

    return m_registeredElements[index].m_element;
  }

  template <class EC>
  bool ElementRegistry<EC>::IsRegistered(const UUID & uuid) const
  {
    for (u32 i = 0; i < m_registeredElementsCount; ++i) {
      if (m_registeredElements[i].m_uuid==uuid)
        return true;
    }
    return false;
  }

  template <class EC>
  bool ElementRegistry<EC>::IsLoaded(const UUID & uuid) const
  {
    for (u32 i = 0; i < m_registeredElementsCount; ++i) {
      const ElementEntry & ee = m_registeredElements[i];
      if (ee.m_uuid==uuid)
        return ee.m_element != 0;
    }
    return false;
  }

  template <class EC>
  const char * ElementRegistry<EC>::AddLibraryPath(const char * path)
  {
    if (!path)
      FAIL(NULL_POINTER);

    LibraryPathString xpath;
    Utils::NormalizePath(path, xpath);
    if (xpath.HasOverflowed())
      return "Library path too long";
    const char * err = Utils::ReadablePath(xpath.GetZString());
    if (err) return err;

    for (u32 i = 0; i < m_libraryPathsCount; ++i)
    {
      if (m_libraryPaths[i].Equals(xpath))
        return 0;
    }

    if (m_libraryPathsCount >= MAX_PATHS)
      return "Too many library paths";

    m_libraryPaths[m_libraryPathsCount] = xpath;
    ++m_libraryPathsCount;
    return 0;
  }

  template <class EC>
  s32 ElementRegistry<EC>::LoadLibraries(UlamClassRegistry<EC> & ucr)
  {
    u32 elementCount = 0;
    for (u32 i = 0; i < m_libraryPathsCount; ++i) {
      s32 ret = LoadLibrary(ucr, m_libraryPaths[i]);
      if (ret < 0) return ret;
      elementCount += (u32) ret;
    }
    return (s32) elementCount;
  }

  template <class EC>
  s32 ElementRegistry<EC>::LoadLibrary(UlamClassRegistry<EC> & ucr, OString256 & libraryPath)
  {
    ElementLibraryLoader<EC> ell;

    const char * err;
    err = ell.Open(libraryPath);
    if (err != 0) {
      LOG.Error("Dynamic library failure on %s", err);
      return -1;
    }

    ElementLibrary<EC> * el = 0;
    err = ell.LoadLibrary(&el);
    if (err != 0) {
      LOG.Error("ElementLibrary not loadable from %s", err);
      return -1;
    }
    u32 count = el->m_elementCount;
    for (u32 i = 0; i < count; ++i) {
      ElementLibraryStub<EC> * els = el->m_elementStubPtrArray[i];
      if (!els)
        FAIL(ILLEGAL_STATE);
      Element<EC> * elt = els->GetElement();
      if (!elt)
        FAIL(ILLEGAL_STATE);
      UUID uuid = elt->GetUUID();
      if (!RegisterElement(*elt))
      {
        LOG.Warning("Already registered: %@, skipping", &uuid);
        continue;
      }
      else
        LOG.Message("Loaded %@ at %p from %s", &uuid, elt, libraryPath.GetZString());

      UlamElement<EC> * uelt = elt->AsUlamElement();
      if (uelt) {
        s32 eret = ucr.RegisterUlamElementEmpty(*uelt);
        if (eret > 0)
          uelt->AllocateEmptyType();
        if (eret < 0 || (eret == 0 && !ucr.RegisterUlamClass(*uelt)))
          LOG.Warning("Ulam Class '%s' already registered", uelt->GetMangledClassName());
      }
    }

    u32 ocount = el->m_otherUlamClassCount;
    for (u32 i = 0; i < ocount; ++i) {
      UlamClass<EC> *ucp = el->m_otherUlamClassPtrArray[i];
      if (!ucp)
        FAIL(ILLEGAL_STATE);
      if (!ucr.RegisterUlamClass(*ucp))
        LOG.Warning("Ulam Class '%s' already registered", ucp->GetMangledClassName());
      else
        LOG.Message("Loaded Ulam Class %s at %p from %s", ucp->GetMangledClassName(), ucp, libraryPath.GetZString());
    }
    return count + ocount;
  }

  template <class EC>
  ElementRegistry<EC>::ElementRegistry()
    : m_registeredElementsCount(0)
    , m_libraryPathsCount(0)
  {

  }

} /* namespace MFM */
