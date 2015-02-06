/* -*- C++ -*- */
#ifndef ELEMENTLIBRARY_H
#define ELEMENTLIBRARY_H

#include "Element.h"

namespace MFM {

  template <class EC>
  struct ElementLibrary {
    u32 m_magic;
    u8 m_version;
    u8 m_reservedFlags;
    u16 m_count;
    u32 m_date;
    u32 m_time;
    Element<EC> ** m_ptrArray;
  };

  enum { ELEMENT_LIBRARY_MAGIC = 0xfea57000 };
  enum { ELEMENT_LIBRARY_VERSION = 1 };

}

#endif // ELEMENTLIBRARY_H
