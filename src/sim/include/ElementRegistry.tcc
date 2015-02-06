/* -*- C++ -*- */
#include <stdlib.h> /* For getenv */
#include <errno.h>  /* For errno */
#include <string.h> /* For strerror */
#include <dirent.h> /* For opendir */
#include "ElementLibraryLoader.h"
#include "Fail.h"

namespace MFM
{

  template <class EC>
  void ElementRegistry<EC>::Init()
  {
    LOG.Debug("Loading ElementLibraries from %d file(s)...", m_libraryPathsCount);
    s32 elements = LoadLibraries();
    if (elements < 0)
    {
      FAIL(ILLEGAL_STATE);
    }
    LOG.Debug("Got %d elements", elements);

#if 0
    FAIL(INCOMPLETE_CODE); // XXXX DEIMPLEMENTED

    /* Scan all element directories for any <UUID>.so files, then
     * register them. */

    LOG.Debug("Searching for elements in %d directories...", m_libraryPathsCount);
    for(u32 i = 0; i < m_libraryPathsCount; i++)
    {
      const char* dirname = m_libraryPaths[i].GetZString();
      LOG.Debug("  Searching %s for shared libs:", dirname);
      DIR* dir = opendir(dirname);

      if(!dir)
      {
        LOG.Warning("  Skipping %s: %s", dirname, strerror(errno));
        continue;
      }

      struct dirent *entry = NULL;

      while((entry = readdir(dir)))
      {
        /* Files and symbolic links are both OK */
        if(entry->d_type == DT_LNK ||
           entry->d_type == DT_REG)
        {
          if(UUID::LegalFilename(entry->d_name))
          {
            LOG.Debug("    ELEMENT FOUND: %s", entry->d_name);


            UUID fileID;
            u32 entrylen = strlen(entry->d_name) - 3; /* For '.so' extension */
            CharBufferByteSource charSource(entry->d_name, entrylen);
            ByteSource& source = charSource;

            fileID.Read(source);

            RegisterUUID(fileID);

            // Update the pathIndex for loading later
            ElementEntry * ee = FindMatching(fileID);
            if (!ee)
              FAIL(ILLEGAL_STATE);
            ee->m_pathIndex = (s32) i;

          }
          else
          {
            LOG.Debug("    Other file: %s", entry->d_name);
          }
        }
      }
      closedir(dir);
    }
#endif
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
  void ElementRegistry<EC>::AddLibraryPath(const char * path)
  {
    if (!path)
      FAIL(NULL_POINTER);

    OString256 xpath;
    xpath = path;
    if (path[0] == '~') {
      const char * home = getenv("HOME");
      if (home) {
        xpath.Reset();
        xpath.Printf("%s%s", home, path + 1);
      }
    }

    for (u32 i = 0; i < m_libraryPathsCount; ++i) {
      if (m_libraryPaths[i].Equals(xpath))
        return;
    }
    if (m_libraryPathsCount >= MAX_PATHS)
      FAIL(OUT_OF_ROOM);
    m_libraryPaths[m_libraryPathsCount] = xpath;
    ++m_libraryPathsCount;
  }

  template <class EC>
  s32 ElementRegistry<EC>::LoadLibraries()
  {
    u32 elementCount = 0;
    for (u32 i = 0; i < m_libraryPathsCount; ++i) {
      s32 ret = LoadLibrary(m_libraryPaths[i]);
      if (ret < 0) return ret;
      elementCount += (u32) ret;
    }
    return (s32) elementCount;
  }

  template <class EC>
  s32 ElementRegistry<EC>::LoadLibrary(OString256 & libraryPath)
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
    u32 count = el->m_count;
    for (u32 i = 0; i < count; ++i) {
      Element<EC> * elt = el->m_ptrArray[i];
      if (!elt)
        FAIL(ILLEGAL_STATE);
      UUID uuid = elt->GetUUID();
      if (!RegisterElement(*elt))
        LOG.Warning("Already registered: %@", &uuid);
      else
        LOG.Message("Loaded %@ at %p from %s", &uuid, elt, libraryPath.GetZString());
    }
    return count;
  }

  template <class EC>
  ElementRegistry<EC>::ElementRegistry()
    : m_registeredElementsCount(0), m_libraryPathsCount(0)
  {

  }

} /* namespace MFM */
