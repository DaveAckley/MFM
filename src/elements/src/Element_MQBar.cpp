#include "Element_MQBar.h"

#if 0 // Sun Feb  1 05:27:33 2015 UNREIMPLEMENTED AS YET IN v3
#ifdef ELEMENT_PLUG_IN

#include "StdEventConfig.h"

extern "C" {
  static void * getref() {
    return (void *) & (MFM::Element_MQBar<MFM::StdEventConfig>::THE_INSTANCE);
  }
  typedef void* (*FuncPtr)();
  FuncPtr get_static_element_pointer[] = { getref };
}

#endif /* ELEMENT_PLUG_IN */
#endif // UNREIMPLEMENTED
