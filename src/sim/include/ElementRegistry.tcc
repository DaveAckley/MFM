/* -*- C++ -*- */
#include <stdlib.h> /* For getenv */
#include <errno.h>  /* For errno */
#include <string.h> /* For strerror */
#include <dirent.h> /* For opendir */

namespace MFM
{

  template <class CC>
  void ElementRegistry<CC>::Init()
  {

    /* Scan all element directories for any <UUID>.so files, then
     * register them. */

    LOG.Debug("Searching for elements in %d directories...", m_searchPathsCount);
    for(u32 i = 0; i < m_searchPathsCount; i++)
    {
      const char* dirname = m_searchPaths[i].GetZString();
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
  }

  template <class CC>
  bool ElementRegistry<CC>::RegisterElement(Element<CC>& e)
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
  Element<CC> * ElementRegistry<CC>::Lookup(const UUID & uuid) const
  {
    const ElementEntry * ee = FindMatching(uuid);
    if (ee) return ee->m_element;
    return 0;
  }

  template <class CC>
  Element<CC> * ElementRegistry<CC>::LookupCompatible(const UUID & uuid) const
  {
    // Try exact match first
    Element<CC> * ret = Lookup(uuid);
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
    PathString xpath;
    xpath = path;
    if (path[0] == '~') {
      const char * home = getenv("HOME");
      if (home) {
        xpath.Reset();
        xpath.Printf("%s%s", home, path + 1);
      }
    }

    for (u32 i = 0; i < m_searchPathsCount; ++i) {
      if (m_searchPaths[i].Equals(xpath))
        return;
    }
    if (m_searchPathsCount >= MAX_PATHS)
      FAIL(OUT_OF_ROOM);
    m_searchPaths[m_searchPathsCount] = xpath;
    ++m_searchPathsCount;
  }

  template <class CC>
  ElementRegistry<CC>::ElementRegistry()
    : m_registeredElementsCount(0), m_searchPathsCount(0)
  {

  }

} /* namespace MFM */
