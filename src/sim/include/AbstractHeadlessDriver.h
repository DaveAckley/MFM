/* -*- C++ -*- */
#ifndef ABSTRACT_HEADLESS_DRIVER_H
#define ABSTRACT_HEADLESS_DRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
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
#include "DriverArguments.h"


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

namespace MFM
{
  /**
   * A class representing a headless driver, i.e. a driver which works
   * only on the command line.
   */
  template<class GC>
  class AbstractHeadlessDriver
  {

  public:

    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    typedef typename CC::ATOM_TYPE T;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

    static const u32 EVENT_WINDOW_RADIUS = R;
    static const u32 GRID_WIDTH = W;
    static const u32 GRID_HEIGHT = H;

    typedef Grid<GC> OurGrid;
    typedef ElementTable<CC> OurElementTable;

    AbstractHeadlessDriver(DriverArguments& args)
    {
      OnceOnly(args);

      m_driverArguments = &args;
    }

    void ReinitUs()
    {

    }

  protected:
    OurGrid& GetGrid()
    {
      return m_grid;
    }

  private:

    char m_simDirBasePath[MAX_PATH_LENGTH];
    u32 m_simDirBasePathLength;
    u32 m_ticksLastStopped;
    u32 m_haltAfterAEPS;

    s32 m_recordEventCountsPerAEPS;
    s32 m_recordTimeBasedDataPerAEPS;

    double m_AEPS;
    double m_AER;

    u32 m_nextEventCountsAEPS;

    OurGrid m_grid;

    DriverArguments* m_driverArguments;

    char* GetSimDirPathTemporary(const char* format, ...)
    {
      va_list ap;
      va_start(ap, format);
      vsnprintf(m_simDirBasePath +
		m_simDirBasePathLength,
		MAX_PATH_LENGTH - 1,
		format, ap);
      return m_simDirBasePath;
    }


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

      m_grid.Unpause();

      /* Sleep at a constant rate for now  */
      /* XXX See what the overhead is here */
      Sleep(1, 1000);

      m_grid.Pause();

      m_AEPS = m_grid.GetTotalEventsExecuted() / m_grid.GetTotalSites();

      LOG.Debug("Elapsed AEPS: %d", (int)m_AEPS);
    }

    void OnceOnly(DriverArguments& args)
    {
      LOG.SetLevel(args.GetInitialLogLevel());

      const char* dirPath = args.GetDataDirPath();
      if(!dirPath)
      {
	dirPath = "/tmp";
      }

      /* Make the main data directory */
      if(mkdir(dirPath, 0777))
      {
	/* It's OK if it already exists */
	if(errno != EEXIST)
	{
	  args.Die("Couldn't make directory '%s' : %s", dirPath, strerror(errno));
	}
      }

      u64 startTime = Utils::GetDateTimeNow();

      snprintf(m_simDirBasePath, MAX_PATH_LENGTH - 1,
	       "%s/%ld", dirPath, startTime);

      m_simDirBasePathLength = strlen(m_simDirBasePath);

      if(m_simDirBasePathLength >= MAX_PATH_LENGTH - MIN_PATH_RESERVED_LENGTH)
      {
	args.Die("Path name too long '%s'", dirPath);
      }

      const char* (subs[]) = { "", "vid", "eps", "tbd", "teps" };
      for(u32 i = 0; i < sizeof(subs) / sizeof(subs[0]); i++)
      {
	const char* path = GetSimDirPathTemporary("%s", subs[i]);
	if(mkdir(path, 0777))
	{
	  args.Die("Couldn't make simulation sub-directory '%s' : %s",
		   path, strerror(errno));
	}
      }

      /* Initialize tbd.txt */
      const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
      FILE* fp = fopen(path, "w");
      fprintf(fp, "#AEPS activesites empty dreg res wall sort-hits"
	          "sort-misses sort-total sort-hit-pctg\n");
      fclose(fp);

      m_AEPS = 0;

      m_recordEventCountsPerAEPS = args.GetRecordEventCountsPerAEPS();
      m_recordTimeBasedDataPerAEPS = args.GetRecordTimeBasedDataPerAEPS();

      u32 seed = args.GetSeed();
      if(!seed)
      {
	seed = time(0);
      }
      SetSeed(seed);

      m_ticksLastStopped = 0;
    }

    void SetSeed(u32 seed)
    {
      if(!seed)
      {
	FAIL(ILLEGAL_ARGUMENT);
      }
      m_grid.SetSeed(seed);
    }

    void ReapplyPostArguments(DriverArguments* argptr)
    {
      DriverArguments& args = *argptr;

      for(u32 i = 0; i < args.GetDisabledTileCount(); i++)
      {
	SPoint& pt = args.GetDisabledTiles()[i];
	m_grid.SetTileToExecuteOnly(pt, false);
      }
    }

    void Reinit()
    {
      ReinitUs();

      m_grid.Reinit();
      m_grid.Needed(Element_Empty<CC>::THE_INSTANCE);

      ReinitPhysics();
      ReinitEden();
    }

    virtual void ReinitEden() = 0;

    virtual void ReinitPhysics() = 0;

    void RunHelper()
    {
      bool running = true;

      while(running)
      {
	Update();

	if(m_haltAfterAEPS > 0 && m_AEPS > m_haltAfterAEPS)
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
