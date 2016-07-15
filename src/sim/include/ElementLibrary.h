/* -*- C++ -*- */
#ifndef ELEMENTLIBRARY_H
#define ELEMENTLIBRARY_H

#include "Element.h"
#include "UlamElement.h"

namespace MFM {
  template <class EC>
  struct ElementLibraryStub {
    virtual ~ElementLibraryStub() { }
    virtual Element<EC> * GetElement() = 0;
  };

  /**
     An ElementLibraryStub for C++ Elements.  C++ Elements do not have
     an analog to UlamElementInfo -- everything is handled directly by
     the specific element subclass code -- so their library stub
     GetElement() method (hopefully, apparently, at the moment)
     doesn't need to do anything.
   */
  template <class EC>
  struct CPPElementLibraryStub : ElementLibraryStub<EC>
  {
    Element<EC> & m_elt;
    CPPElementLibraryStub(Element<EC> & eltr)
      : m_elt(eltr)
    {
    }

    Element<EC> * GetElement()
    {
      return &m_elt;
    }
  };


  /**
     An ElementLibraryStub specifically for UlamElements
   */
  template <class EC>
  struct UlamElementLibraryStub : ElementLibraryStub<EC>
  {
    UlamElement<EC> & m_elt;
    const UlamElementInfo<EC> * m_info;
    UlamElementLibraryStub(UlamElement<EC> & eltr, UlamElementInfo<EC> * infop)
      : m_elt(eltr), m_info(infop)
    {
    }

    Element<EC> * GetElement()
    {
      if (m_info)
        m_elt.SetInfo(m_info);
      return &m_elt;
    }
  };

  /**
     An ElementLibraryStub specifically for the UlamElement Empty
   */
  template <class EC>
  struct UlamEmptyElementLibraryStub : ElementLibraryStub<EC>
  {
    UlamElement<EC> & m_elt;
    const UlamElementInfo<EC> * m_info;
    UlamEmptyElementLibraryStub(UlamElement<EC> & eltr, UlamElementInfo<EC> * infop)
      : m_elt(eltr), m_info(infop)
    {
      // Give the ulam Empty the official empty type
      m_elt.AllocateEmptyType();
      //Element_Empty<EC>::THE_INSTANCE;
    }

    Element<EC> * GetElement()
    {
      FAIL(ILLEGAL_STATE);
      return 0;
    }
  };

  template <class EC>
  struct ElementLibrary {
    u32 m_magic;
    u8 m_version;
    u8 m_subversion;
    u16 m_reservedFlags;
    u32 m_date;
    u32 m_time;
    u32 m_elementCount;
    ElementLibraryStub<EC> ** m_elementStubPtrArray;
    u32 m_otherUlamClassCount;
    UlamClass<EC> ** m_otherUlamClassPtrArray;
  };

  enum { ELEMENT_LIBRARY_MAGIC = 0xfea57000 };
  enum {
    ELEMENT_LIBRARY_VERSION = 2,
    ELEMENT_LIBRARY_SUBVERSION = 0
  };

}

#endif // ELEMENTLIBRARY_H
