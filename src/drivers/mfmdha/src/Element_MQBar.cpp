#include "Element_MQBar.h"

#ifdef ELEMENT_PLUG_IN

#include "LocalConfig.h"

extern "C" {
  static MFM::Element<MFM::OurCoreConfig> * (elementPtrArray[]) = { &MFM::Element_MQBar<MFM::OurCoreConfig>::THE_INSTANCE };
  static MFM::ElementLibrary<MFM::OurCoreConfig> el = {
    MFM::ELEMENT_LIBRARY_MAGIC,
    MFM::ELEMENT_LIBRARY_VERSION,
    0,
    1,
    MFM_BUILD_DATE,
    MFM_BUILD_TIME,
    elementPtrArray
  };
  void * get_static_element_library_pointer =  &el;
}

#endif /* ELEMENT_PLUG_IN */
