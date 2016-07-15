/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "UlamClassRegistry.h"

namespace MFM {

  template <class EC> class UlamElement; //forward
  template <class EC> class UlamClass; //forward

  template <class EC>
  UlamContext<EC>::UlamContext(const UlamContext<EC>& cxref) 
    : m_elementTable(cxref.m_elementTable) { }

  template <class EC>
  const Element<EC> * UlamContext<EC>::LookupElementTypeFromContext(u32 etype) const
  {
    return m_elementTable.Lookup(etype);
  } //LookupElementTypeFromContext

  template <class EC>
  const UlamClass<EC> * UlamContext<EC>::LookupUlamElementTypeFromContext(u32 etype) const
  {
    const Element<EC> * eltptr = this->LookupElementTypeFromContext(etype);
    if (!eltptr) return NULL;
    const UlamElement<EC> * ueltptr =  eltptr->AsUlamElement();
    return ueltptr; //might be NULL
  } //LookupUlamElementTypeFromContext

} //MFM
