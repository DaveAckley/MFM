#include "Element_QBar.h"

#ifdef ELEMENT_PLUG_IN

#include "LocalConfig.h"

extern "C" {
  static void * getref() {
    return (void *) & (MFM::Element_QBar<MFM::OurCoreConfig>::THE_INSTANCE);
  }
  typedef void* (*FuncPtr)();
  FuncPtr get_static_element_pointer[] = { getref };
}

#endif /* ELEMENT_PLUG_IN */
