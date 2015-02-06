#include "Element_QBar.h"

#ifdef ELEMENT_PLUG_IN

#include "StdEventConfig.h"

extern "C" {
  static void * getref() {
    return (void *) & (MFM::Element_QBar<MFM::StdEventConfig>::THE_INSTANCE);
  }
  typedef void* (*FuncPtr)();
  FuncPtr get_static_element_library_pointer[] = { getref };
}

#endif /* ELEMENT_PLUG_IN */
