/*                                              -*- mode:C++ -*-
  AbstractDriver.h Base class for all MFM drivers
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file AbstractDriver.h Base class for all MFM drivers
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTDRIVER_H
#define ABSTRACTDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/time.h>  /* for gettimeofday */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "ExternalConfig.h"
#include "ExternalConfigFunctions.h"
#include "FileByteSource.h"
#include "FileByteSink.h"
#include "itype.h"
#include "Grid.h"
#include "ElementTable.h"
#include "VArguments.h"
#include "StdElements.h"
#include "ElementRegistry.h"
#include "Version.h"


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

#define INITIAL_AEPS_PER_FRAME 2

namespace MFM
{
  /**
   * An abstract driver from which all MFM drivers should inherit.
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

    typedef ElementRegistry<CC> OurElementRegistry;
    typedef StdElements<CC> OurStdElements;
    typedef Grid<GC> OurGrid;
    typedef ElementTable<CC> OurElementTable;

    void Sleep(u32 seconds, u64 nanos)
    {
      struct timespec tspec;
      tspec.tv_sec = seconds;
      tspec.tv_nsec = nanos;

      nanosleep(&tspec, NULL);
    }

    void RunGrid(OurGrid& grid)
    {
      const s32 ONE_THOUSAND = 1000;
      const s32 ONE_MILLION = ONE_THOUSAND*ONE_THOUSAND;

      grid.Unpause();  // pausing and unpausing should be overhead!

      u32 startMS = GetTicks();  // So get the ticks after unpausing
      if (m_ticksLastStopped != 0)
        m_msSpentOverhead += startMS - m_ticksLastStopped;
      else
        m_msSpentOverhead = 0;

      Sleep(m_microsSleepPerFrame/ONE_MILLION, (u64) (m_microsSleepPerFrame%ONE_MILLION)*ONE_THOUSAND);
      m_ticksLastStopped = GetTicks(); // and before pausing

      grid.Pause();

      m_msSpentRunning += (m_ticksLastStopped - startMS);

      m_AEPS = grid.GetTotalEventsExecuted() / grid.GetTotalSites();
      m_AER = 1000 * (m_AEPS / m_msSpentRunning);

      m_overheadPercent = 100.0*m_msSpentOverhead/(m_msSpentRunning+m_msSpentOverhead);

      double diff = m_AEPS - m_lastFrameAEPS;
      double err = MIN(1.0, MAX(-1.0, m_aepsPerFrame - diff));

      // Correct up to 20% of current each frame
      m_microsSleepPerFrame = (100+20*err)*m_microsSleepPerFrame/100;
      m_microsSleepPerFrame = MIN(100000000, MAX(1000, m_microsSleepPerFrame));

      m_lastFrameAEPS = m_AEPS;

      ExportEventCounts(grid);

      PostUpdate();
    }

    void DecrementAEPSPerFrame()
    {
      if(m_aepsPerFrame > 1)
      {
	m_aepsPerFrame--;
      }
    }

    void IncrementAEPSPerFrame()
    {
      if(m_aepsPerFrame < 1000)
      {
	m_aepsPerFrame++;
      }
    }

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

    /**
     * Called at the end of the Reinit() call. This is for custom
     * initialization behavior.
     */
    virtual void PostReinit(VArguments& args)
    { }

    /**
     * Establish the Garden of Eden configuration on the grid.
     */
    virtual void ReinitEden() = 0;

    /**
     * Register any element types needed for the run.
     */
    virtual void ReinitPhysics() = 0;

    /**
     * To be run at the end of a frame update.
     */
    virtual void PostUpdate()
    { }

    /**
     * To be run during first initialization, only once. This runs
     * after all standard argument parsing and is available to extend
     * that behavior.  Any overrides of this method should be certain
     * to call Super::OnceOnly (probably at the beginning, but in any
     * case, sometime) during their execution, so more abstract levels
     * can do any processing they need to.
     */
    virtual void OnceOnly(VArguments& args)
    {
      if(!args.Appeared("-d"))
      {
	SetDataDirFromArgs(NULL, this);
      }

      LOG.Message("Writing to simulation directory '%s'", GetSimDirPathTemporary(""));

      const char* (subs[]) = { "", "vid", "eps", "tbd", "teps", "save" };
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

      m_elementRegistry.AddPath("/home/sixstring982/Documents/Git/MFMv2/bin");
      m_elementRegistry.AddPath("./bin");
      m_elementRegistry.Init();
    }

    /**
     * The main loop which runs this simulation.
     */
    virtual void RunHelper()
    {
      bool running = true;

      while(running)
      {
	RunGrid(m_grid);

	if(m_haltAfterAEPS > 0 && m_AEPS > m_haltAfterAEPS)
	{
	  running = false;
	}
      }
    }

  private:

    OurElementRegistry m_elementRegistry;
    OurStdElements m_se;
    OurGrid m_grid;

    u32 m_ticksLastStopped;
    u32 m_haltAfterAEPS;

    u64 m_startTimeMS;
    u64 m_msSpentRunning;
    u64 m_msSpentOverhead;
    s32 m_microsSleepPerFrame;
    double m_overheadPercent;
    double m_lastFrameAEPS;
    u32 m_aepsPerFrame;

    s32 m_recordEventCountsPerAEPS;
    s32 m_recordTimeBasedDataPerAEPS;

    double m_AEPS;
    double m_AER;

    u32 m_nextEventCountsAEPS;

    VArguments m_varguments;

    const char* m_configurationPath;

    char m_simDirBasePath[MAX_PATH_LENGTH];
    u32 m_simDirBasePathLength;

    u32 GetTicks()
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);

      /* ms since epoch */
      u64 ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

      return  ms - m_startTimeMS;
    }

    /***********************************
     *      MANDATORY VARGUMENTS
     ***********************************/

    static void PrintArgUsage(const char* not_needed, void* vargs)
    {
      VArguments& args = *((VArguments*)vargs);
      args.Usage();
    }

    static void PrintVersion(const char* not_needed, void* nullForShort)
    {
      fprintf(stderr, "%s\n", nullForShort ? MFM_VERSION_STRING_LONG : MFM_VERSION_STRING_SHORT);
      exit(0);
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
	       "%s/%ld/", dirPath, startTime);

      driver.m_simDirBasePathLength = strlen(driver.m_simDirBasePath);

      if(driver.m_simDirBasePathLength >= MAX_PATH_LENGTH - MIN_PATH_RESERVED_LENGTH)
      {
	args.Die("Path name too long '%s'", dirPath);
      }
    }

    static void RegisterElementPath(const char* path, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);

      driver.m_elementRegistry.AddPath(path);
    }

    static void SetHaltAfterAEPSFromArgs(const char* aeps, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);

      driver.m_haltAfterAEPS = atoi(aeps);
    }

    static void LoadFromConfigFile(const char* path, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);

      driver.m_configurationPath = path;
    }

    void LoadFromConfigurationPath()
    {
      if(m_configurationPath)
      {
	LOG.Debug("Loading configuration from %s...", m_configurationPath);

	ExternalConfig<GC> cfg(GetGrid());
	RegisterExternalConfigFunctions<GC>(cfg);
	FileByteSource fs(m_configurationPath);

	cfg.SetByteSource(fs, m_configurationPath);

	cfg.Read();

	fs.Close();
      }
    }

    void ExportEventCounts(OurGrid& grid)
    {
      if (m_recordEventCountsPerAEPS > 0) {
	if (m_AEPS > m_nextEventCountsAEPS) {

	  const char * path = GetSimDirPathTemporary("eps/%010d.ppm", m_nextEventCountsAEPS);
	  FILE* fp = fopen(path, "w");
          FileByteSink fbs(fp);
	  grid.WriteEPSImage(fbs);
	  fclose(fp);

	  path = GetSimDirPathTemporary("teps/%010d-average.ppm", m_nextEventCountsAEPS);
	  fp = fopen(path, "w");
          FileByteSink fbs2(fp);
	  grid.WriteEPSAverageImage(fbs2);
	  fclose(fp);

	  m_nextEventCountsAEPS += m_recordEventCountsPerAEPS;
	}
      }
    }

  public:

    AbstractDriver() :
      m_grid(m_elementRegistry),
      m_ticksLastStopped(0),
      m_startTimeMS(0),
      m_msSpentRunning(0),
      m_msSpentOverhead(0),
      m_microsSleepPerFrame(1000),
      m_aepsPerFrame(INITIAL_AEPS_PER_FRAME),
      m_AEPS(0),
      m_configurationPath(NULL)
    {
    }

    void Init(u32 argc, const char** argv)
    {
      AddDriverArguments();

      SetSeed(1);

      m_varguments.ProcessArguments(argc, argv);

      m_startTimeMS = GetTicks();

      OnceOnly(m_varguments);
    }

    VArguments & GetVArguments()
    {
      return m_varguments;
    }

    void RegisterArgument(const char* description, const char* filter,
			  VArgumentHandleValue func, void* handlerArg,
			  bool runFunc)
    {
      m_varguments.RegisterArgument(description, filter, func, handlerArg, runFunc);
    }

    void RegisterSection(const char* sectionLabel)
    {
      m_varguments.RegisterSection(sectionLabel);
    }

    /**
     * Adds any command-line arguments desired, by calling
     * #RegisterArgument with appropriate arguments.  Note:
     * (Sub-)subclasses of AbstractDriver should \e always override
     * this method, even if they do not wish to add command-line
     * arguments, and begin their overriding method with
     * Super::AddDriverArguments().  Such chaining lets all levels of
     * abstraction define command-line arguments, with the most
     * abstract going first.
     */
    virtual void AddDriverArguments()
    {
      RegisterSection("General switches");

      RegisterArgument("Display this help message, then exit.",
                       "-h|--help", &PrintArgUsage, (void*)(&m_varguments), false);

      RegisterArgument("Amount of logging output is ARG (0 -> none, 8 -> max)",
                       "-l|--log", &SetLoggingLevel, NULL, true);

      RegisterArgument("Print the brief version number, then exit.",
                       "-v|--version", &PrintVersion, NULL, false);

      RegisterArgument("Print the full version number, then exit.",
                       "-V|--Version", &PrintVersion, this, false);

      RegisterArgument("Set master PRNG seed to ARG (u32)",
                       "-s|--seed", &SetSeedFromArgs, this, true);

      RegisterArgument("Record event counts every ARG aeps",
                       "-e|--events", &SetRecordEventCountsFromArgs, this, true);

      RegisterArgument("Records time based data every ARG aeps",
                       "-t|--timebd",
                       &SetRecordTimeBasedDataFromArgs, this, true);

      RegisterArgument("If ARG > 0, Halts after ARG elapsed aeps.",
                       "--haltafteraeps", &SetHaltAfterAEPSFromArgs, this, true);

      RegisterArgument("Store data in per-sim directories under ARG (string)",
                       "-d|--dir", &SetDataDirFromArgs, this, true);

      RegisterArgument("Add ARG as a path to search for element libraries",
		       "-ep|--elementpath", &RegisterElementPath, this, true);

      RegisterArgument("Load initial configuration from file at path ARG (string)",
		       "-cp|--configurationPath", &LoadFromConfigFile, this, true);
    }


    virtual void ReinitUs()
    { }

    u32 GetHaltAfterAEPS()
    {
      return m_haltAfterAEPS;
    }

    double GetAEPS()
    {
      return m_AEPS;
    }

    double GetAER()
    {
      return m_AER;
    }

    void SetAER(double aer)
    {
      m_AER = aer;
    }

    u32 GetAEPSPerFrame()
    {
      return m_aepsPerFrame;
    }

    double GetOverheadPercent()
    {
      return m_overheadPercent;
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

    void Reinit()
    {
      m_lastFrameAEPS = 0;

      ReinitUs();

      m_grid.Reinit();

      m_grid.Needed(Element_Empty<CC>::THE_INSTANCE);

      ReinitPhysics();

      ReinitEden();

      PostReinit(m_varguments);

      LoadFromConfigurationPath();
    }

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

#endif /* ABSTRACTDRIVER_H */
