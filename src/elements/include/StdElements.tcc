/* -*- C++ -*- */
#include "StdElementsHeaders.inc"

namespace MFM {
  template <class EC>
  StdElements<EC>::StdElements() : m_stdElements(0) {
#define XX(eltName) \
    LOG.Debug("StdElement #%d: %@",++m_stdElements, & eltName<EC>::THE_INSTANCE.GetUUID())
#include "StdElements.inc"
#undef XX
  }
}
