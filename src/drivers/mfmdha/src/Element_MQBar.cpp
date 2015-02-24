#include "Element_MQBar.h"

#ifdef ELEMENT_PLUG_IN

#include "StdEventConfig.h"
#include "ElementLibrary.h"

extern "C" {
  static MFM::CPPElementLibraryStub<MFM::StdEventConfig> _cppels10(MFM::Element_MQBar<MFM::StdEventConfig>::THE_INSTANCE);

  static MFM::ElementLibraryStub<MFM::StdEventConfig> * (_elementStubPtrArray_[]) = {
    &_cppels10
  };
  static MFM::ElementLibrary<MFM::StdEventConfig> el = {
    MFM::ELEMENT_LIBRARY_MAGIC,
    MFM::ELEMENT_LIBRARY_VERSION,
    0,
    1,
    MFM_BUILD_DATE,
    MFM_BUILD_TIME,
    _elementStubPtrArray_
  };
  void * MFM_ELEMENT_LIBRARY_LOADER_SYMBOL =  &el;
}

#endif /* ELEMENT_PLUG_IN */
