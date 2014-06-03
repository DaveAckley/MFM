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
   * only on the command line.
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

  private:

    void Sleep(u32 seconds, u64 nanos)
    {
      struct timespec tspec;
      tspec.tv_sec = seconds;
      tspec.tv_nsec = nanos;

      nanosleep(&tspec, NULL);
    }

    void Update()
    {
      //const s32 ONE_THOUSAND = 1000;
      //const s32 ONE_MILLION = ONE_THOUSAND * ONE_THOUSAND;

      AbstractDriver<GC>::GetGrid().Unpause();

      /* Sleep at a constant rate for now  */
      /* XXX See what the overhead is here */
      Sleep(1, 1000);

      AbstractDriver<GC>::GetGrid().Pause();

      Super::SetAEPS(AbstractDriver<GC>::GetGrid().GetTotalEventsExecuted() /
		     AbstractDriver<GC>::GetGrid().GetTotalSites());

      LOG.Debug("Elapsed AEPS: %d", (int)Super::GetAEPS());
    }

    /* No extra behavior */
    virtual void PostReinit(VArguments& args)
    {  }

    void RunHelper()
    {
      bool running = true;

      while(running)
      {
	Update();

	if(Super::GetHaltAfterAEPS() > 0 && Super::GetAEPS() > Super::GetHaltAfterAEPS())
	{
	  running = false;
	}
      }
    }

  public:
    void Run()
    {
      unwind_protect
      ({
	 MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
	 fprintf(stderr, "Failure reached top-level! Aborting\n");
	 abort();
       },
       {
	 RunHelper();
       });
    }
  };
}

#endif /* ABSTRACT_HEADESS_DRIVER_H */
