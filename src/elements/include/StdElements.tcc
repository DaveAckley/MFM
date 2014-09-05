/* -*- C++ -*- */
#include "StdElementsHeaders.inc"

namespace MFM {
  template <class CC>
  StdElements<CC>::StdElements() : m_stdElements(0) {
#define XX(eltName) \
    LOG.Debug("StdElement #%d: %@",++m_stdElements, & eltName<CC>::THE_INSTANCE.GetUUID())
#include "StdElements.inc"
#undef XX
  }
}
