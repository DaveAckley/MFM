/* -*- C++ -*- */
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"
#include "Site.h"
//#include "UlamClass.h"
//#include "UlamElement.h"

namespace MFM {

  template <class EC> class UlamElement; //forward
  template <class EC> class UlamClass; //forward

  template <class EC>
  UlamContextRestricted<EC>::UlamContextRestricted(const UlamContextRestricted<EC>& cxref) 
    : UlamContext<EC>(cxref.m_elementTable) 
    , m_ucr(cxref.m_ucr)
  { }

} //MFM
