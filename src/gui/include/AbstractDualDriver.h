/* -*- C++ -*- */
#ifndef ABSTRACT_DUAL_DRIVER_H
#define ABSTRACT_DUAL_DRIVER_H


#ifdef MFM_GUI_DRIVER
#include "AbstractGUIDriver.h"
#define DUAL_DRIVER_TYPE AbstractGUIDriver
#else
#include "AbstractHeadlessDriver.h"
#define DUAL_DRIVER_TYPE AbstractHeadlessDriver
#endif

namespace MFM
{
  /**
   * A quantum driver, existing both as an AbstractGUIDriver and an
   * AbstractHeadlessDriver. If the symbol MFM_GUI_DRIVER is set, this
   * will build as a GUI driver. If not, this will build as a headless
   * driver.
   */
  template<class GC>
  class AbstractDualDriver : public DUAL_DRIVER_TYPE<GC>
  {
  private:
    typedef DUAL_DRIVER_TYPE<GC> Super;

  protected:

    AbstractDualDriver(u32 argc, const char** argv) : Super(argc, argv)
    { }
  };
}

#endif /* ABSTRACT_DUAL_DRIVER_H */
