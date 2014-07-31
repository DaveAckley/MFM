#include "Element_MQBar.h"

#ifdef ELEMENT_PLUG_IN

#include "LocalConfig.h"

extern "C" {
  static void * getref() {
    return (void *) & (MFM::Element_MQBar<MFM::OurCoreConfig>::THE_INSTANCE);
  }
  typedef void* (*FuncPtr)();
  FuncPtr get_static_element_pointer[] = { getref };
}

#endif /* ELEMENT_PLUG_IN */
