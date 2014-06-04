/* -*- C++ -*- */
#ifndef ABSTRACT_HEADLESS_DRIVER_H
#define ABSTRACT_HEADLESS_DRIVER_H

#include "Logger.h"
#include "FileByteSink.h"
#include "itype.h"
#include "ParamConfig.h"
#include "GridConfig.h"
#include "CoreConfig.h"
#include "Tile.h"
#include "Grid.h"
#include "ElementTable.h"
#include "Element_Empty.h" /* Need common elements */
#include "VArguments.h"
#include "AbstractDriver.h"

namespace MFM
{
  /**
   * A class representing a headless driver, i.e. a driver which works
   * only on the command line without input.
   */
  template<class GC>
  class AbstractHeadlessDriver : public AbstractDriver<GC>
  {
  protected:

    typedef AbstractDriver<GC> Super;
    typedef typename Super::OurGrid OurGrid;
    typedef typename Super::CC CC;

    AbstractHeadlessDriver(u32 argc, const char** argv) :
      AbstractDriver<GC>(argc, argv)
    { }

    virtual void PostUpdate()
    {
      LOG.Debug("AEPS: %d", (u32)Super::GetAEPS());
    }
  };
}

#endif /* ABSTRACT_HEADESS_DRIVER_H */
