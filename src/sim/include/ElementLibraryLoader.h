/* -*- C++ -*- */
#ifndef ELEMENTLIBRARYLOADER_H
#define ELEMENTLIBRARYLOADER_H
#include <dlfcn.h>
#include <stdlib.h>
#include <iostream>
#include "Element.h"
#include "ElementLibrary.h"
#include "OverflowableCharBufferByteSink.h"

#define MFM_ELEMENT_LIBRARY_LOADER_SYMBOL mfm_get_static_element_library_pointer

namespace MFM {

  template <class EC>
  class ElementLibraryLoader {
  public:

    ElementLibraryLoader() : m_dlHandle(0) { }

    const char * Open(ZStringableByteSink & path)
    {
       m_dlHandle = dlopen(path.GetZString(), RTLD_LAZY|RTLD_GLOBAL);

       const char * err = dlerror();
       if (!m_dlHandle) {
         if (!err)
           err = "Unspecified dlopen error";
         return err;
       }
       return 0;
    }

    const char * LoadLibrary(ElementLibrary<EC> ** elpp)
    {
      if (!m_dlHandle)
        FAIL(ILLEGAL_STATE);

      // get accessor
      // clear dlerror
      dlerror();

      LOG.Debug("Calling dlsym");
      void * symptr = dlsym(m_dlHandle, XSTR_MACRO(MFM_ELEMENT_LIBRARY_LOADER_SYMBOL));
      LOG.Debug("Back from dlsym");

      const char* dlsym_error = dlerror();
      if (dlsym_error)
      {
        return dlsym_error;
      }

      ElementLibrary<EC> ** elp = reinterpret_cast<ElementLibrary<EC> **>(symptr);
      if (!elp || !*elp)
      {
        return "Accessor to ElementLibrary failed";
      }

      ElementLibrary<EC> * el = *elp;
      if (el->m_magic != ELEMENT_LIBRARY_MAGIC)
      {
        return "Bad ElementLibrary magic number";
      }

      if (el->m_version > ELEMENT_LIBRARY_VERSION)
      {
        return "ElementLibrary version newer than ours";
      }

      *elpp = el;
      return 0;
    }

    ~ElementLibraryLoader()
    {
      if (m_dlHandle)
        dlclose(m_dlHandle);
      m_dlHandle = 0;
    }

  private:
    void * m_dlHandle;
  };
}

#endif // ELEMENTLIBRARYLOADER_H
