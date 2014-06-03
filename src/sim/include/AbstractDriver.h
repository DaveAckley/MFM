/* -*- C++ -*- */
#ifndef ABSTRACTDRIVER_H
#define ABSTRACTDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "itype.h"
#include "Grid.h"
#include "ElementTable.h"
#include "VArguments.h"


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

namespace MFM
{
  /**
   * An abstract driver which all MFM drivers should inherit.
   */
  template<class GC>
  class AbstractDriver
  {
  protected:
    // Extract short type names
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

  private:
    OurGrid m_grid;

    u32 m_ticksLastStopped;
    u32 m_haltAfterAEPS;

    s32 m_recordEventCountsPerAEPS;
    s32 m_recordTimeBasedDataPerAEPS;

    bool m_startPaused;

    double m_AEPS;
    double m_AER;

    u32 m_nextEventCountsAEPS;

    VArguments m_varguments;

    char m_simDirBasePath[MAX_PATH_LENGTH];
    u32 m_simDirBasePathLength;

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

    /***********************************
     *      MANDATORY VARGUMENTS
     ***********************************/

    static void PrintArgUsage(const char* not_needed, void* vargs)
    {
      VArguments& args = *((VArguments*)vargs);
      args.Usage();
    }

    static void SetLoggingLevel(const char* level, void* not_needed)
    {
      LOG.SetLevel(atoi(level));
    }

    static void SetSeedFromArgs(const char* seedstr, void* driver)
    {
      u32 seed = atoi(seedstr);
      if(!seed)
      {
	seed = time(0);
      }
      ((AbstractDriver*)driver)->SetSeed(seed);
    }

    static void SetRecordEventCountsFromArgs(const char* aepsStr, void* driver)
    {
      u32 recAEPS = atoi(aepsStr);
      ((AbstractDriver*)driver)->m_recordEventCountsPerAEPS = recAEPS;
    }

    static void SetRecordTimeBasedDataFromArgs(const char* tbdStr, void* driver)
    {
      u32 tbdAEPS = atoi(tbdStr);
      ((AbstractDriver*)driver)->m_recordTimeBasedDataPerAEPS = tbdAEPS;
    }

    static void SetDataDirFromArgs(const char* dirPath, void* driverPtr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverPtr);
      VArguments& args = driver.m_varguments;

      if(!dirPath || strlen(dirPath) == 0)
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

      snprintf(driver.m_simDirBasePath, MAX_PATH_LENGTH - 1,
	       "%s/%ld", dirPath, startTime);

      driver.m_simDirBasePathLength = strlen(driver.m_simDirBasePath);

      if(driver.m_simDirBasePathLength >= MAX_PATH_LENGTH - MIN_PATH_RESERVED_LENGTH)
      {
	args.Die("Path name too long '%s'", dirPath);
      }
    }

    static void SetStartPausedFromArgs(const char* not_used, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);

      driver.m_startPaused = true;
    }

    static void SetHaltAfterAEPSFromArgs(const char* not_used, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);

      driver.m_haltAfterAEPS = atoi(not_used);
    }

    virtual void AddMandatoryDriverArguments(VArguments& args)
    {
      args.RegisterArgument("Display this help message, then exit.",
			    "-h|--help", &PrintArgUsage, (void*)(&args), false);

      args.RegisterArgument("Amount of logging output (0 is none, 8 is high)",
			    "-l|--log", &SetLoggingLevel, NULL, true);

      args.RegisterArgument("Set master PRNG seed to NUM (u32)",
			    "-s|--seed", &SetSeedFromArgs, this, true);

      args.RegisterArgument("Record event counts every AEPS aeps",
			    "-e|--events", &SetRecordEventCountsFromArgs, this, true);

      args.RegisterArgument("Records time based data every AEPS aeps",
			    "-t|--timebd",
			    &SetRecordTimeBasedDataFromArgs, this, true);

      args.RegisterArgument("Starts paused to allow display configuration.",
			    "--startpaused", &SetStartPausedFromArgs, this, false);

      args.RegisterArgument("If AEPS > 0, Halts after AEPS elapsed aeps.",
			    "--haltafteraeps", &SetHaltAfterAEPSFromArgs, this, true);

      /*
       * Placing a newline at the end of this description makes any arguments added
       * after this one appear to be in their own section.
       */
      args.RegisterArgument("Store data in per-sim directories under DIR\n",
			    "-d|--dir", &SetDataDirFromArgs, this, true);

    }

    void OnceOnly(VArguments& args)
    {
      if(!args.Appeared("-d"))
      {
	SetDataDirFromArgs(NULL, this);
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

      PostOnceOnly(args);
    }


  public:

    AbstractDriver(u32 argc, const char** argv) :
      m_ticksLastStopped(0),
      m_startPaused(true),
      m_AEPS(0)
    {
      AddMandatoryDriverArguments(m_varguments);
      AddDriverArguments(m_varguments);

      m_varguments.ProcessArguments(argc, argv);

      OnceOnly(m_varguments);
    }

    bool GetStartPaused()
    {
      return m_startPaused;
    }

    u32 GetHaltAfterAEPS()
    {
      return m_haltAfterAEPS;
    }

    double GetAEPS()
    {
      return m_AEPS;
    }

    void SetAEPS(double val)
    {
      m_AEPS = val;
    }

    OurGrid & GetGrid()
    {
      return m_grid;
    }

    void SetSeed(u32 seed)
    {
      if(!seed)
      {
	FAIL(ILLEGAL_ARGUMENT);
      }
      m_grid.SetSeed(seed);
    }

    virtual void ReinitUs()
    {

    }

    /**
     * Called at the end of the Reinit() call. This is for custom
     * initialization behavior.
     */
    virtual void PostReinit(VArguments& args) = 0;

    /**
     * Establish the Garden of Eden configuration on the grid.
     */
    virtual void ReinitEden() = 0;

    /**
     * Register any element types needed for the run.
     */
    virtual void ReinitPhysics() = 0;

    virtual void AddDriverArguments(VArguments& args)
    { }

    /**
     * To be run during first initialization, only once. This runs
     * after all standard argument parsing and is meant to be used to
     * extend this behavior.
     */
    virtual void PostOnceOnly(VArguments& args)
    { }

    void Reinit()
    {
      ReinitUs();

      m_grid.Reinit();

      m_grid.Needed(Element_Empty<CC>::THE_INSTANCE);

      ReinitPhysics();

      ReinitEden();

      PostReinit(m_varguments);
    }

  };
}

#endif /* ABSTRACTDRIVER_H */
