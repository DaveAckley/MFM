#include "Element_QBar.h"

#ifdef ELEMENT_PLUG_IN

#include "LocalConfig.h"

extern "C" {
  void * get_static_element_pointer() {
    return (void *) & (MFM::Element_QBar<MFM::OurCoreConfig>::THE_INSTANCE);
  }
}

#endif /* ELEMENT_PLUG_IN */
