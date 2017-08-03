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
#include "Util.h"
#include "Utils.h"     /* for GetDateTimeNow, Sleep */
#include "ExternalConfig.h"
#include "ExternalConfigSectionDriver.h"
#include "ExternalConfigSectionGrid.h"
#include "OverflowableCharBufferByteSink.h"
#include "FileByteSource.h"
#include "FileByteSink.h"
#include "TeeByteSink.h"
#include "itype.h"
#include "Grid.h"
#include "ElementTable.h"
#include "VArguments.h"
#include "StdElements.h"
#include "ElementRegistry.h"
#include "Version.h"
#include "DebugTools.h"


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

#define MAX_NEEDED_ELEMENTS 100
#define MAX_CONFIGURATION_PATHS 32

#define INITIAL_AEPS_PER_FRAME 1

namespace MFM
{
  /**
   * An abstract driver from which all MFM drivers should
   * inherit. This should be the highest level structure and contain
   * just about everything needed to run a successful simulation.
   */
  template<class GC>
  class AbstractDriver
  {
  public:

    /**
     * The EventConfig used to describe templated pieces of the core
     * components.
     */
    typedef typename GC::EVENT_CONFIG EC;

    /**
     * The AtomConfig used to configure several templated pieces of
     * the core components.
     */
    typedef typename EC::ATOM_CONFIG AC;

    /**
     * The type of every Atom used in this simulation.
     */
    typedef typename AC::ATOM_TYPE T;

    /**
     * Template shortcut for an ElementRegistry with the correct
     * template parameters.
     */
    typedef ElementRegistry<EC> OurElementRegistry;

    /**
     * Template shortcut for an instance of StdElements with the
     * correct template parameters.
     */
    typedef StdElements<EC> OurStdElements;

    /**
     * Template shortcut for a Grid with the correct template
     * parameters.
     */
    typedef Grid<GC> OurGrid;

    /**
     * Template shortcut for an ElementTable with the correct template
     * parameters.
     */
    typedef ElementTable<EC> OurElementTable;

    /**
     * Exported from the GridConfiguration, the enumerated size of
     * every EventWindow used by this simulation.
     */
    enum { EVENT_WINDOW_RADIUS = EC::EVENT_WINDOW_RADIUS};

    /**
     * The width of the Grid used by this simulation.
     */
    const u32 GRID_WIDTH;

    /**
     * The height of the Grid used by this simulation.
     */
    const u32 GRID_HEIGHT;

    void NeedElement(Element<EC>* element)
    {
      for (u32 i = 0; i < m_neededElementCount; ++i)
      {
        if (m_neededElements[i] == element)
          return;

        if (m_neededElements[i]->GetUUID() == element->GetUUID())
          FAIL(ILLEGAL_STATE);
      }

      if(m_neededElementCount >= MAX_NEEDED_ELEMENTS)
      {
        FAIL(OUT_OF_ROOM);
      }

      m_neededElements[m_neededElementCount++] = element;
    }

    virtual void WriteTimeBasedCustomHeader(FileByteSink& fp)
    { }

    virtual void WriteTimeBasedCustomData(FileByteSink& fp)
    { }

    void WriteTimeBasedData(FileByteSink& fp, bool exists)
    {

      if(!exists)
      {
        fp.Printf("# AEPS AEPS/Frame AER100 Overhead100");
        for(u32 i = 0; i < m_neededElementCount; i++)
        {
          fp.WriteByte(' ');
          for(const char* p = m_neededElements[i]->GetName(); *p; p++)
          {
            if(isspace(*p))
            {
              fp.WriteByte('_');
            }
            else
            {
              fp.WriteByte(*p);
            }
          }
        }
        WriteTimeBasedCustomHeader(fp);
        fp.Println();
      }

      fp.Print((u64)GetAEPS());
      fp.WriteByte(' ');
      fp.Print(GetAEPSPerFrame());
      fp.WriteByte(' ');
      fp.Print((u64)(100.0 * GetAER()));
      fp.WriteByte(' ');
      fp.Print((u64)(100.0 * GetOverheadPercent()));

      for(u32 i = 0; i < m_neededElementCount; i++)
      {
        fp.WriteByte(' ');
        fp.Print((u32)GetGrid().GetAtomCount(m_neededElements[i]->GetType()));
      }

      WriteTimeBasedCustomData(fp);
      fp.Println();
    }

    void WriteTimeBasedData()
    {
      const char* path = GetSimDirPathTemporary("tbd/data.dat");
      bool exists = true;
      {
        FILE* fp = fopen(path, "r");
        if (!fp)
        {
          exists = false;
        }
        else
        {
          fclose(fp);
        }
      }
      FILE* fp = fopen(path, "a");
      FileByteSink fbs(fp);

      WriteTimeBasedData(fbs, exists);
      fclose(fp);
    }

    /**
     * Runs the held Grid and all its associated threads for a brief
     * amount of time, letting about \c m_aepsPerFrame AEPS occur
     * before pausing. This also learns about how long an AEPS takes
     * to elapse, making subsequent calls more accurate in duration.
     *
     * @param grid The Grid which is updated during this call.
     */
    void UpdateGrid(OurGrid& grid)
    {
      grid.Unpause();  // pausing and unpausing should be overhead!

      u64 startMS = GetTicks();  // So get the ticks after unpausing
      if (m_ticksLastStopped != 0)
        m_msSpentOverhead += startMS - m_ticksLastStopped;
      else
        m_msSpentOverhead = 0;

      SleepUsec(m_microsSleepPerFrame);

      m_ticksLastStopped = GetTicks(); // and before pausing

      grid.Pause();

      u32 thisPeriodMS = m_ticksLastStopped - startMS;
      m_msSpentRunning += thisPeriodMS;

      if (thisPeriodMS == 0) {
        LOG.Warning("Zero ms in sample");
        thisPeriodMS = 1;
      }

      double thisAERsample;

#if 0 /* site based stats not ready for prime time */
      {
        // Trying for site-based statistics, rather that event-based
        // statistics, because of the variable ew boundary sizes.
        u64 totalAccesses = grid.GetTotalSitesAccessed();
        u32 totalSites = grid.GetTotalSites();
        m_AEPS = totalAccesses / ((double) totalSites);
        m_AER = 1000 * (m_AEPS / m_msSpentRunning);

        u64 newEvents = totalAccesses - m_lastTotalEvents;
        m_lastTotalEvents = totalAccesses;
        thisAERsample = 1000.0 * newEvents / totalSites / thisPeriodMS;
      }
#endif
      {
        // Traditional event-based statistics, whether that event hit
        // one site or forty-one.
        u64 totalEvents = grid.GetTotalEventsExecuted();
        u32 totalSites = grid.GetTotalSites();
        m_AEPS = totalEvents / ((double) totalSites);
        m_AER = 1000 * (m_AEPS / m_msSpentRunning);

        u64 newEvents = totalEvents - m_lastTotalEvents;

        // Fri Jul 22 04:25:11 2016 .mfs files currently don't store
        // m_lastTotalEvents, leading to ridiculous AER spikes that
        // take a _long_ time to average out.  Let's hackety-whack try
        // to reset here if we seem offkilter.
        if (m_lastTotalEvents > totalEvents ||
            totalEvents > m_lastTotalEvents + 1000 * totalSites)
          newEvents = 0;

        m_lastTotalEvents = totalEvents;
        thisAERsample = 1000.0 * newEvents / totalSites / thisPeriodMS;
      }

      const double BACKWARDS_AVERAGE_RATE = 0.99;
      m_recentAER = BACKWARDS_AVERAGE_RATE * m_recentAER +
                    (1 - BACKWARDS_AVERAGE_RATE) * thisAERsample;

      m_overheadPercent = 100.0*m_msSpentOverhead/(m_msSpentRunning+m_msSpentOverhead);

      double diff = m_AEPS - m_lastFrameAEPS;
      double err = MIN(5.0, MAX(-5.0, m_aepsPerFrame - diff));

      // Correct up to 10% of current each frame
      m_microsSleepPerFrame = (100+2*err)*m_microsSleepPerFrame/100;
      m_microsSleepPerFrame = MIN(100000000, MAX(1000, m_microsSleepPerFrame));

      LOG.Debug("diff=%d err100=%d usleep=%d",
                (s32) diff,
                (s32) (err*100),
                m_microsSleepPerFrame);

      m_lastFrameAEPS = m_AEPS;

      CheckEpochProcessing(grid);

      PostUpdate();
    }

    /**
     * Reduce the \c m_aepsPerFrame (or, the number of AEPS which
     * should elapse every call to \c UpdateGrid() ), keeping it above
     * \c 0 at all times.
     *
     * @param amount The amount of AEPS per frame to decrement by.
     */
    void DecreaseAEPSPerFrame()
    {
      u32 decr = 0;
      if(m_aepsPerFrame > 100) decr = 50;
      else if (m_aepsPerFrame > 10) decr = 5;
      else if (m_aepsPerFrame > 1) decr = 1;
      m_aepsPerFrame -= decr;
    }

    /**
     * Increase the \c m_aepsPerFrame (or, the number of AEPS which
     * should elapse every call to \c UpdateGrid() ), keeping it
     * below \c 1000 at all times.
     *
     * @param amount The amount of AEPS per frame to increment by.
     */
    void IncreaseAEPSPerFrame()
    {
      u32 incr = 0;
      if (m_aepsPerFrame < 10) incr = 1;
      else if (m_aepsPerFrame < 100) incr = 5;
      else if (m_aepsPerFrame < 1000) incr = 50;
      m_aepsPerFrame += incr;
    }

    /**
     * Called at the end of the Reinit() call. This is for custom
     * initialization behavior.
     *
     * @param args The VArguments , which should have been gotten from
     *             the command line, which will be processed again
     *             during this call.
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
    void ReinitPhysics()
    {
      for(u32 i = 0; i < m_neededElementCount; i++)
      {
        GetGrid().Needed(*m_neededElements[i]);
      }

      PostReinitPhysics();
    }

    /**
     * Used by GUI Drivers to register Elements in places needed.
     */
    virtual void PostReinitPhysics()
    { }

    /**
     * To be defined by the top level driver only. This allows all
     * Elements to be known by this AbstractDriver before they are
     * inserted into its sub-drivers .
     */
    virtual void DefineNeededElements() = 0;

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
     *
     * @param args The VArguments , which should have been gotten from
     *             the command line. These allow this method to
     *             configure itself properly from the command line
     *             arguments.
     */
    virtual void OnceOnly(VArguments& args)
    {
      if(!args.Appeared("-d"))
      {
        SetDataDirFromArgs(NULL, this);
      }
      LOG.Message("Writing to simulation directory '%s'", GetSimDirPathTemporary(""));

      if (!this->m_suppressStdElements)
      {
        const char * LIBUECORE_PATH = "elements/libuecore.so";
        OString512 buffer;
        if (!Utils::GetReadableResourceFile(LIBUECORE_PATH, buffer))
        {
          LOG.Error("Can't find standard elements file '%s'", LIBUECORE_PATH);
        }
        else
        {
          m_elementRegistry.AddLibraryPath(buffer.GetZString());
        }
      }

      if (this->m_includeUEDemos)
      {
        const char * LIBUEDEMOS_PATH = "elements/libuedemos.so";
        OString512 buffer;
        if (!Utils::GetReadableResourceFile(LIBUEDEMOS_PATH, buffer))
        {
          LOG.Error("Can't find ulam demo elements file '%s'", LIBUEDEMOS_PATH);
        }
        else
        {
          m_elementRegistry.AddLibraryPath(buffer.GetZString());
        }
      }

      const char* (subs[]) =
      {
        "", "vid", "eps", "tbd", "teps", "save", "screenshot", "autosave", "log"
      };

      for(u32 i = 0; i < sizeof(subs) / sizeof(subs[0]); i++)
      {
        const char* path = GetSimDirPathTemporary("%s", subs[i]);
        if(mkdir(path, 0777))
        {
          args.Die("Couldn't make simulation sub-directory '%s' : %s",
                   path, strerror(errno));
        }
      }

      m_elementRegistry.Init(m_grid.GetUlamClassRegistry());
      u32 dlcount = m_elementRegistry.GetRegisteredElementCount();
      for (u32 i = 0; i < dlcount; ++i)
      {
        NeedElement(m_elementRegistry.GetRegisteredElement(i));
      }

      DefineNeededElements();
    }

    /**
     * The main loop which runs this simulation -- unless overridden
     * by a subclass.
     */
    virtual void RunHelper()
    {
      bool running = true;

      while(running)
      {
        UpdateGrid(m_grid);
        running = RunHelperExiter();
      }

    }

    virtual bool RunHelperExiter() {
      double full = m_grid.GetFullSitePercentage();
      if((m_haltAfterAEPS > 0 && m_AEPS > m_haltAfterAEPS)
         || (m_haltOnEmpty && full == 0.0)
         || (m_haltOnFull && full == 1.0)
         || (m_AEPS > 0 && m_haltOnExtinctionOf && 
             m_grid.GetAtomCountFromSymbol(m_extinctionSymbol)==0)
         )
      {
        // Free final save if halting on --halt*.  Hope for good-looking corpse.
        {
          const char* filename =
            GetSimDirPathTemporary("save/final-%D-%D.mfs", m_epochCount, (u32) m_AEPS);
          SaveGrid(filename);
        }
        WriteTimeBasedData();
        m_grid.ShutdownTileThreads();
        return false;
      }
      return true;
    }

    void SaveGridWithConstantFilename(const char* filename)
    {
      const char* finalName =
        GetSimDirPathTemporary("%s", filename);
      SaveGrid(finalName);
    }

    void SetAEPSPerEpoch(u32 aeps)
    {
      if (m_maxEpochLength > 0 && aeps > m_maxEpochLength)
        aeps =  m_maxEpochLength;
      m_AEPSPerEpoch = aeps;
    }

    u32 GetAEPSPerEpoch() const
    {
      return m_AEPSPerEpoch;
    }

    void InitTicks(u64 priorTicks)
    {
      m_totalPriorTicks = priorTicks;
      m_currentTickBasis = GetTicksSinceEpoch();
    }

    u64 GetTicksSinceEpoch() const
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);

      /* cast since what's a time_t */
      return ((u64) tv.tv_sec) * 1000 + tv.tv_usec / 1000;
    }

    u64 GetTicks() const
    {
      return
        GetTicksSinceEpoch()
        - m_currentTickBasis
        + m_totalPriorTicks;
    }

    /***********************************
     *      MANDATORY VARGUMENTS
     ***********************************/

    static void PrintArgUsage(const char* not_needed, void* vargs)
    {
      VArguments& args = *((VArguments*)vargs);
      args.Usage();
    }

    static void SelectDemoFromArg(const char* demo, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      bool wantList = (strcmp("list", demo) == 0);
      bool wantAll = (strcmp("all", demo) == 0);

      // Look for demos-list file
      OString512 buf;
      const char * demoListFile = "elements/demos.dat";
      if (!Utils::GetReadableResourceFile(demoListFile, buf))
      {
        args.Die("Cannot find '%s'", demoListFile);
      }
      else
      {
        FileByteSource fs(buf.GetZString());
        if (!fs.IsOpen())
        {
          args.Die("Can't open '%s'",buf.GetZString());
        }

        u32 lastMatches;
        u32 count = 0;

        while (true)
        {
          OString64 name;
          OString256 mfz, libue, classes, info;

          lastMatches = 
            fs.Scanf("%Z%Z%Z%Z%Z\n", &name, &mfz, &libue, &classes, &info);

          if (lastMatches != 6)
            break;

          if (wantList)
          {
            const char * zname = name.GetZString();
            printf("\nDEMO: %s\n", zname);
            printf(" To run the demo standalone: mfzrun %s demo\n", zname);
            printf(" To load the demo's classes: mfms --demo %s\n", zname);
            printf("   includes classes: %s\n", 
                   classes.GetZString());
            ++count;
          }
          else if (wantAll || !strcmp(demo,name.GetZString()))
          {
            printf("Including %s from %s\n", 
                   classes.GetZString(),
                   name.GetZString());

            // fake up an appropriate -ep call
            RegisterElementLibraryPath(libue.GetZString(), driverptr);
            ++count;
          }

        }

        fs.Close();

        if (lastMatches != 0) 
        {
          LOG.Warning("Incomplete or corrupt %s", buf.GetZString());
        }

        if ((wantList || wantAll) && count == 0) 
          args.Die("No demos found");

        if (wantList)
        {
          exit(0);
        }

        if (!wantList && !wantAll && count == 0)
          args.Die("Demo '%s' not found, try '--demo list' for a list", demo);
      }
    }


    static void PrintVersion(const char* not_needed, void* nullForShort)
    {
      fprintf(stderr, "%s\n", nullForShort ? MFM_VERSION_STRING_LONG : MFM_VERSION_STRING_SHORT);
      exit(0);
    }

    static void SetLoggingLevel(const char* level, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 val = Logger::ParseLevel(level);
      if (val < 0)
        args.Die("'%s' not recognized as a logging level", level);
      LOG.SetLevel((Logger::Level) val);
    }

    static const char * GetNumberFromString(const char* str, s32 & output, s32 min, s32 max)
    {
      MFM_API_ASSERT_NONNULL(str);
      if (*str == 0) return "Empty argument";

      char * ez;
      s32 num = strtol(str, &ez, 0);
      if (*str != 0 && *ez == 0) {
        if (num < min) return "Numeric argument too small";
        if (num > max) return "Numeric argument too large";
        output = num;
        return NULL;
      }
      return "Non-numeric in argument";
    }

    static void SetSeedFromArgs(const char* seedstr, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = GetNumberFromString(seedstr, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad seed '%s': %s", seedstr, errmsg);
      }

      u32 seed = (u32) out;
      if(!seed)
      {
        seed = time(0);
      }
      driver.SetSeed(seed);
    }

    static void SetAEPSPerEpochFromArgs(const char* aepsStr, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(aepsStr, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad AEPS '%s': %s", aepsStr, errmsg);
      }

      driver.SetAEPSPerEpoch((u32) out);
    }

    static void SetAutosavePerEpochsFromArgs(const char* arg, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(arg, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad autosave per epochs '%s': %s", arg, errmsg);
      }

      driver.m_autosavePerEpochs = (u32) out;
    }

    static void SetPicturesPerRateFromArgs(const char* aeps, void* driverptr)
    {
      AbstractDriver& driver = *(AbstractDriver*)driverptr;
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(aeps, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad accelerate '%s': %s", aeps, errmsg);
      }

      driver.m_accelerateAfterEpochs = out;
    }

    static void SetSurgePerEpochFromArgs(const char* aeps, void* driverptr)
    {
      AbstractDriver& driver = *(AbstractDriver*)driverptr;
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(aeps, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad surged '%s': %s", aeps, errmsg);
      }

      driver.m_surgeAfterEpochs = out;

      if (driver.m_surgeAfterEpochs > 0)   // If actually surging
      {                                    // it'll happen immediately
        driver.m_acceleration = 0;         // so push acceleration back
      }
    }

    static void SetMaxEpochLengthFromArgs(const char* aeps, void* driverptr)
    {
      AbstractDriver& driver = *(AbstractDriver*)driverptr;
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(aeps, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad max epoch length '%s': %s", aeps, errmsg);
      }

      driver.m_maxEpochLength = out;
    }

    static void SetHaltOnEmpty(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_haltOnEmpty = 1;
    }

    static void SetHaltOnFull(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_haltOnFull = 1;
    }

    static void SetNoStdFromArgs(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_suppressStdElements = 1;
    }

    static void SetUEDemosFromArgs(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_includeUEDemos = 1;
    }

    static void SetCppDemosFromArgs(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_includeCPPDemos = 1;
    }

    static void SetGridImages(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_gridImages = 1;
    }

    static void SetTileImages(const char* not_needed, void* driver)
    {
      ((AbstractDriver*)driver)->m_tileImages = 1;
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
               "%s/%d%06d/", dirPath,
               Utils::GetDateFromDateTime(startTime),
               Utils::GetTimeFromDateTime(startTime));

      driver.m_simDirBasePathLength = strlen(driver.m_simDirBasePath);

      if(driver.m_simDirBasePathLength >= MAX_PATH_LENGTH - MIN_PATH_RESERVED_LENGTH)
      {
        args.Die("Path name too long '%s'", dirPath);
      }
    }

    static void RegisterElementLibraryPath(const char* path, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      const char * result = driver.m_elementRegistry.AddLibraryPath(path);
      if (result)
      {
        args.Die("Bad element library path '%s': %s", path, result);
      }
    }

    static void IgnoreComment(const char* kv, void* driverptr)
    {
      // A Good Job Well Done!
    }

    static void RegisterKeyValue(const char* kv, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      u32 key;
      s32 value;
      char dummy;
      if (sscanf(kv,"%u:%d%c",&key,&value,&dummy) != 2)
        args.Die("-kv argument must be a number pair like '0:3', not '%s'", kv);
      driver.m_grid.SetTileParameter(key, value);
    }

    static void SetHaltAfterAEPSFromArgs(const char* aeps, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(aeps, out, 0, S32_MAX);
      if (errmsg)
      {
        args.Die("Bad AEPS '%s': %s", aeps, errmsg);
      }

      driver.m_haltAfterAEPS = (u32) out;
    }

    static void SetHaltOnExtinctionOfFromArgs(const char* symbol, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      if (strlen(symbol) > 2)
        args.Die("Bad atomic symbol '%s'", symbol);

      driver.m_extinctionSymbol[0] = symbol[0];
      driver.m_extinctionSymbol[1] = symbol[1];
      driver.m_extinctionSymbol[2] = symbol[2];
      driver.m_haltOnExtinctionOf = true;
    }

    static void SetWarpFactorFromArgs(const char* wfs, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(wfs, out, 0, 10);
      if (errmsg)
      {
        args.Die("Warp factor '%s' not in 0..10: %s", wfs, errmsg);
      }

      driver.m_grid.SetWarpFactor(out);
    }

    static void LoadFromConfigFile(const char* path, void* driverptr)
    {
      AbstractDriver& driver = *((AbstractDriver*)driverptr);
      VArguments& args = driver.m_varguments;

      if (driver.m_configurationPathCount >= MAX_CONFIGURATION_PATHS)
      {
        args.Die("Too many configuration paths, max %d, for '%s'",
                 MAX_CONFIGURATION_PATHS,
                 path);
      }
      driver.m_configurationPaths[driver.m_configurationPathCount] = path;
      ++driver.m_configurationPathCount;
    }

    void CheckEpochProcessing(OurGrid& grid)
    {
      if (m_AEPSPerEpoch >= 0 || m_accelerateAfterEpochs > 0 || m_surgeAfterEpochs > 0)
      {
        if (m_AEPS >= m_nextEpochAEPS)
        {
          DoEpochEvents(grid, m_epochCount, m_nextEpochAEPS);
          m_nextEpochAEPS += m_AEPSPerEpoch;
          ++m_epochCount;
        }
      }
    }

  public:
    /**
     * Gets a formatted string representing a working path to the
     * assets directory of this simulation. This is where the local
     * copy of the \c res/ directory is kept and should be used for
     * finding any assets in that location.
     *
     * @param format The formatting string used to parse the arguments
     *               that follow it into a reasonable format.
     *
     * @returns \c format , with the location of the local resources
     *          directory prepended to it.
     */
    const char* GetSimDirPathTemporary(const char* format, ...) const
    {
      static OString512 buf;
      buf.Reset();
      buf.Printf("%s",m_simDirBasePath);
      va_list ap;
      va_start(ap, format);
      buf.Vprintf(format, ap);
      if (buf.HasOverflowed())
      {
        FAIL(OUT_OF_ROOM);
      }
      return buf.GetZString();
    }

    void SaveRuntimeData(ByteSink & sink) const
    {
      sink.Print(this->GetTicks(), Format::LXX64); // current ticks becomes prior ticks on load
      sink.Print(m_msSpentRunning, Format::LXX64);
      sink.Print(m_msSpentOverhead, Format::LXX64);
      sink.Print(m_microsSleepPerFrame, Format::LXX32);
      sink.Print((u64)(m_overheadPercent * 100000), Format::LXX64);
      sink.Print((u64)(m_lastFrameAEPS * 100000), Format::LXX64);
      sink.Print(m_aepsPerFrame, Format::LXX32);
      sink.Printf(",");

      sink.Print(m_AEPSPerEpoch, Format::LXX32);
      sink.Print(m_autosavePerEpochs, Format::LXX32);
      sink.Print(m_accelerateAfterEpochs, Format::LXX32);
      sink.Print(m_acceleration, Format::LXX32);
      sink.Print(m_surgeAfterEpochs, Format::LXX32);
      sink.Print(m_maxEpochLength, Format::LXX32);
      sink.Printf(",");

      sink.Print(m_gridImages, Format::LXX32);
      sink.Print(m_tileImages, Format::LXX32);
      sink.Printf(",");

      sink.Print((u64)(m_AEPS * 100000), Format::LXX64);
      sink.Print((u64)(m_AER * 100000), Format::LXX64);
      sink.Print((u64)(m_recentAER * 100000), Format::LXX64);
      sink.Print(m_lastTotalEvents,Format::LXX64);
      sink.Printf(",");

      sink.Print(m_nextEpochAEPS,Format::LXX32);
      sink.Print(m_epochCount,Format::LXX32);
    }

    bool LoadRuntimeData(LineCountingByteSource & source)
    {
      u64 tmp_GetTicks;
      u64 tmp_m_msSpentRunning;
      u64 tmp_m_msSpentOverhead;
      u32 tmp_m_microsSleepPerFrame;
      u64 tmp_m_overheadPercent;
      u64 tmp_m_lastFrameAEPS;
      u32 tmp_m_aepsPerFrame;
      if (!source.Scan(tmp_GetTicks, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_msSpentRunning, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_msSpentOverhead, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_microsSleepPerFrame, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_overheadPercent, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_lastFrameAEPS, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_aepsPerFrame, Format::LXX32)) return false;
      if (1 != source.Scanf(",")) return false;

      u32 tmp_m_AEPSPerEpoch;
      u32 tmp_m_autosavePerEpochs;
      u32 tmp_m_accelerateAfterEpochs;
      u32 tmp_m_acceleration;
      u32 tmp_m_surgeAfterEpochs;
      u32 tmp_m_maxEpochLength;

      if (!source.Scan(tmp_m_AEPSPerEpoch, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_autosavePerEpochs, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_accelerateAfterEpochs, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_acceleration, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_surgeAfterEpochs, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_maxEpochLength, Format::LXX32)) return false;
      if (1 != source.Scanf(",")) return false;

      u32 tmp_m_gridImages;
      u32 tmp_m_tileImages;

      if (!source.Scan(tmp_m_gridImages, Format::LXX32)) return false;
      if (!source.Scan(tmp_m_tileImages, Format::LXX32)) return false;
      if (1 != source.Scanf(",")) return false;

      u64 tmp_m_AEPS;
      u64 tmp_m_AER;
      u64 tmp_m_recentAER;
      u64 tmp_m_lastTotalEvents;

      if (!source.Scan(tmp_m_AEPS, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_AER, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_recentAER, Format::LXX64)) return false;
      if (!source.Scan(tmp_m_lastTotalEvents,Format::LXX64)) return false;
      if (1 != source.Scanf(",")) return false;

      u32 tmp_m_nextEpochAEPS;
      u32 tmp_m_epochCount;

      if (!source.Scan(tmp_m_nextEpochAEPS,Format::LXX32)) return false;
      if (!source.Scan(tmp_m_epochCount,Format::LXX32)) return false;

      //// success begins

      m_totalPriorTicks = tmp_GetTicks;
      m_msSpentRunning = tmp_m_msSpentRunning;
      m_msSpentOverhead = tmp_m_msSpentOverhead;
      m_microsSleepPerFrame = tmp_m_microsSleepPerFrame;
      m_overheadPercent = tmp_m_overheadPercent / 100000.0;
      m_lastFrameAEPS = tmp_m_lastFrameAEPS / 100000.0;
      m_aepsPerFrame = tmp_m_aepsPerFrame;

      m_AEPSPerEpoch = tmp_m_AEPSPerEpoch;
      m_autosavePerEpochs = tmp_m_autosavePerEpochs;
      m_accelerateAfterEpochs = tmp_m_accelerateAfterEpochs;
      m_acceleration = tmp_m_acceleration;
      m_surgeAfterEpochs = tmp_m_surgeAfterEpochs;
      m_maxEpochLength = tmp_m_maxEpochLength;

      m_gridImages = tmp_m_gridImages;
      m_tileImages = tmp_m_tileImages;

      m_AEPS = tmp_m_AEPS / 100000.0;
      m_AER = tmp_m_AER / 100000.0;
      m_recentAER = tmp_m_recentAER / 100000.0;
      m_lastTotalEvents = tmp_m_lastTotalEvents;

      m_nextEpochAEPS = tmp_m_nextEpochAEPS;
      m_epochCount = tmp_m_epochCount;

      return true;
    }

    void AutosaveGrid(u32 epochs)
    {
      const char* filename =
        GetSimDirPathTemporary("autosave/%D-%D.mfs", epochs, (u32) m_AEPS);
      SaveGrid(filename);
    }

    ExternalConfig<GC> & GetExternalConfig()
    {
      return m_externalConfig;
    }

    void SaveGrid(const char* filename)
    {

      LOG.Message("Saving to: %s", filename);
      FILE* fp = fopen(filename, "w");
      FileByteSink fs(fp);

      m_externalConfig.Write(fs);
      fs.Close();
    }

    void LoadFromConfigurationPath()
    {
      if (m_configurationPathCount > 0)
      {
        ++m_currentConfigurationPath;
        if (m_currentConfigurationPath >= m_configurationPathCount)
        {
          m_currentConfigurationPath = 0;
        }
        ReloadCurrentConfigurationPath();
      }
    }

    void ReloadCurrentConfigurationPath()
    {
      if(m_configurationPathCount == 0)
      {
        return;
      }

      const char * path = m_configurationPaths[m_currentConfigurationPath];

      LoadMFS(path);
    }

    bool LoadMFS(const char * path)
    {
      OString512 buf;
      // Accept absolute paths and check resource dirs for relative paths
      if (path[0] == '/' || !Utils::GetReadableResourceFile(path, buf))
      {
        buf.Printf("%s",path); // absolute path or not resource relative
      } 
      /* else buf filled with resource path */

      LOG.Message("Loading configuration '%s'", buf.GetZString());

      FileByteSource fs(buf.GetZString());
      if (fs.IsOpen())
      {
        m_externalConfig.SetByteSource(fs, buf.GetZString());
        m_externalConfig.Read();
        fs.Close();
        LOG.Message("Loaded configuration '%s'", buf.GetZString());
        return true;
      }

      LOG.Error("Can't read configuration file '%s'", buf.GetZString());
      return false;
    }


    /**
     * Method to do end-of-epoch processing.  Base class recounts the
     * grid and handles --gridImage and --tileImage processing here,
     * so all subclasses should override this method and do
     * Super::DoEpochEvents to ensure all methods are called.
     */
    virtual void DoEpochEvents(OurGrid& grid, u32 epochs, u32 epochAEPS)
    {
      LOG.Debug("Epoch %d: %d AEPS", epochs, epochAEPS);

      WriteTimeBasedData();

      if (m_gridImages)
      {
        const char * path = GetSimDirPathTemporary("eps/%010d.ppm", epochAEPS);
        FILE* fp = fopen(path, "w");
        FileByteSink fbs(fp);
        grid.WriteEPSImage(fbs);
        fclose(fp);
      }

      if (m_tileImages)
      {
        const char * path = GetSimDirPathTemporary("teps/%010d-average.ppm", epochAEPS);
        FILE* fp = fopen(path, "w");
        FileByteSink fbs2(fp);
        grid.WriteEPSAverageImage(fbs2);
        fclose(fp);
      }

      if (m_autosavePerEpochs > 0 && (epochs % m_autosavePerEpochs) == 0)
      {
        this->AutosaveGrid(epochs);
      }

      if (m_accelerateAfterEpochs > 0 && (epochs % m_accelerateAfterEpochs) == 0)
      {
        this->SetAEPSPerEpoch(MAX(1u, this->GetAEPSPerEpoch() + m_acceleration));
      }

      if (m_accelerateAfterEpochs > 0 &&
          m_surgeAfterEpochs > 0 &&
          (epochs % m_surgeAfterEpochs) == 0)
      {
        ++m_acceleration;
      }


    }

    AbstractDriver(u32 gridWidth, u32 gridHeight)
      : GRID_WIDTH(gridWidth)
      , GRID_HEIGHT(gridHeight)
      , m_neededElementCount(0)
      , m_grid(m_elementRegistry, GRID_WIDTH, GRID_HEIGHT)
      , m_ticksLastStopped(0)
      , m_totalPriorTicks(0)
      , m_currentTickBasis(0)
      , m_haltAfterAEPS(0)
      , m_haltOnExtinctionOf(false) // if true, m_extinctionSymbol has (unvalidated) content
      , m_haltOnEmpty(false)
      , m_haltOnFull(false)
      , m_suppressStdElements(false)
      , m_includeUEDemos(false)
      , m_includeCPPDemos(false)
      , m_msSpentRunning(0)
      , m_msSpentOverhead(0)
      , m_microsSleepPerFrame(1000)
      , m_overheadPercent(0.0)
      , m_lastFrameAEPS(INITIAL_AEPS_PER_FRAME)
      , m_aepsPerFrame(INITIAL_AEPS_PER_FRAME)
      , m_AEPSPerEpoch(100)
      , m_autosavePerEpochs(10)
      , m_accelerateAfterEpochs(0)
      , m_acceleration(1)
      , m_surgeAfterEpochs(0)
      , m_maxEpochLength(0)
      , m_gridImages(false)
      , m_tileImages(false)
      , m_AEPS(0.0)
      , m_AER(0.0)
      , m_recentAER(0)
      , m_lastTotalEvents(0)
      , m_nextEpochAEPS(0)
      , m_epochCount(0)
      , m_configurationPathCount(0)
      , m_currentConfigurationPath(U32_MAX)
      , m_simDirBasePathLength(0)
      , m_externalConfig(*this)
      , m_externalConfigSectionDriver(m_externalConfig, *this)
      , m_externalConfigSectionGrid(m_externalConfig, m_grid)
    {
      InitTicks(0); // Overwritten later on -cp load
    }

    virtual void RegisterExternalConfigSections()
    {
      m_externalConfig.RegisterSection(m_externalConfigSectionDriver);
      m_externalConfig.RegisterSection(m_externalConfigSectionGrid);
    }

    void SaveCommandLine(u32 argc, const char** argv)
    {
      m_commandLineArguments.Reset();
      for (u32 i = 0; i < argc; ++i) {
        if (i) m_commandLineArguments.Printf(" ");
        m_commandLineArguments.Printf("%s",argv[i]);
      }
      m_commandLineArguments.GetZString(); // null terminate
    }

    const char * GetCommandLine() const
    {
      return m_commandLineArguments.GetBuffer();
    }

    void AddInternalLogging()
    {
      const char* path = this->GetSimDirPathTemporary("log/log.txt");
      LOG.Message("Logging to: %s", path);
      FILE* fp = fopen(path, "w");
      if (!fp) FAIL(IO_ERROR);
      {
        static FileByteSink fbs(fp, true);
        static TeeByteSink logT;

        ByteSink * old = LOG.SetByteSink(logT);
        logT.SetSink1(old);
        logT.SetSink2(&fbs);
      }
      LOG.Message("Added log target: %s", path);
      LOG.Message("Command line: %s", this->GetCommandLine());
    }

    void ProcessArguments(u32 argc, const char** argv)
    {
      RegisterExternalConfigSections();

      SaveCommandLine(argc,argv);

      AddDriverArguments();

      SetSeed(1);

      m_varguments.ProcessArguments(argc, argv);

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

    void RegisterArgumentSection(const char* sectionLabel)
    {
      m_varguments.RegisterArgumentSection(sectionLabel);
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
      RegisterArgumentSection("General switches");

      RegisterArgument("Display this help message, then exit.",
                       "-h|--help", &PrintArgUsage, (void*)(&m_varguments), false);

      RegisterArgument("Show built-in demos (--demo list), or load one (--demo NAME) or all (--demo all)",
                       "--demo", &SelectDemoFromArg, this, true);

      RegisterArgument("Amount of logging output is ARG (0 -> none, 8 -> max)",
                       "-l|--log", &SetLoggingLevel, NULL, true);

      RegisterArgument("Print the brief version number, then exit.",
                       "-v|--version", &PrintVersion, NULL, false);

      RegisterArgument("Print the full version number, then exit.",
                       "-V|--Version", &PrintVersion, this, false);

      RegisterArgument("Set master PRNG seed to ARG (u32)",
                       "-s|--seed", &SetSeedFromArgs, this, true);

      RegisterArgument("Set epoch length to ARG AEPS",
                       "-e|--epoch", &SetAEPSPerEpochFromArgs, this, true);

      RegisterArgument("Autosave grid every ARG epochs (default 10; 0 for never)",
                       "-a|--autosave", &SetAutosavePerEpochsFromArgs, this, true);

      RegisterArgument("Increase the epoch length every ARG epochs",
                             "--accelerate",
                             &SetPicturesPerRateFromArgs, this, true);

      RegisterArgument("Increase the epoch length acceleration every ARG epochs",
                             "--surge",
                             &SetSurgePerEpochFromArgs, this, true);

      RegisterArgument("Set the max epoch length ARG (caps --accelerate and --surge)",
                             "--maxepochlength",
                             &SetMaxEpochLengthFromArgs, this, true);

      RegisterArgument("Each epoch, write grid AEPS image to per-sim eps/ directory",
                       "--gridImages", &SetGridImages, this, false);

      RegisterArgument("Each epoch, write tile AEPS image to per-sim teps/ directory",
                       "--tileImages", &SetTileImages, this, false);

      RegisterArgument("If ARG > 0, Halts after ARG elapsed aeps.",
                       "--haltafteraeps", &SetHaltAfterAEPSFromArgs, this, true);

      RegisterArgument("Halts after element symbol ARG is absent from the grid.",
                       "--haltifextinct", &SetHaltOnExtinctionOfFromArgs, this, true);

      RegisterArgument("Halts if grid is empty.",
                       "--haltonempty", &SetHaltOnEmpty, this, false);

      RegisterArgument("Halts if grid is full.",
                       "--haltonfull", &SetHaltOnFull, this, false);

      RegisterArgument("Store data in per-sim directories under ARG (string)",
                       "-d|--dir", &SetDataDirFromArgs, this, true);

      RegisterArgument("Suppress loading core ulam elements (DReg, etc)",
                       "--no-std", &SetNoStdFromArgs, this, false);

      RegisterArgument("Include some Ulam demo elements",
                       "--ue-demos", &SetUEDemosFromArgs, this, false);

      RegisterArgument("Include (older) C++ demo elements (City, etc)",
                       "--cpp-demos", &SetCppDemosFromArgs, this, false);

      RegisterArgument("Add ARG as the path to an element library (.so)",
                       "-ep|--elementpath", &RegisterElementLibraryPath, this, true);

      RegisterArgument("Load initial configuration from file at path ARG (string)",
                       "-cp|--configpath", &LoadFromConfigFile, this, true);

      RegisterArgument("Set warp factor 0..10 (0: flattest space; 10: highest AER)",
                       "-wf|--warpfactor", &SetWarpFactorFromArgs, this, true);

      RegisterArgument("Add a key=value pair to simulation parameters (string)",
                       "-kv|--keyvalue", &RegisterKeyValue, this, true);

      RegisterArgument("Command line comment, logged but otherwise ignored (string)",
                       "-#|--comment", &IgnoreComment, this, true);

    }


    virtual void ReinitUs()
    { }

    u32 GetHaltAfterAEPS()
    {
      return m_haltAfterAEPS;
    }

    const char * GetHaltAfterExinctionOfSymbol()
    {
      if (!m_haltOnExtinctionOf)
        FAIL(ILLEGAL_STATE);
      
      return m_extinctionSymbol;
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

    u64 GetMsSpentRunning() const
    {
      return m_msSpentRunning;
    }

    void SetMsSpentRunning(u64 ms)
    {
      m_msSpentRunning = ms;
    }

    double GetRecentAER()
    {
      return m_recentAER;
    }

    void SetRecentAER(double aer)
    {
      m_recentAER = aer;
    }

    u32 GetAEPSPerFrame()
    {
      return m_aepsPerFrame;
    }

    void SetAEPSPerFrame(u32 aepsPerFrame)
    {
      m_aepsPerFrame = aepsPerFrame;
    }

    double GetOverheadPercent()
    {
      return m_overheadPercent;
    }

    void SetOverheadPercent(double overheadPercent)
    {
      m_overheadPercent = overheadPercent;
    }

    OurGrid & GetGrid() { return m_grid; }
    const OurGrid & GetGrid() const { return m_grid; }

    void SetSeed(u32 seed)
    {
      if(!seed)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      m_grid.SetSeed(seed);
    }

    void Init()
    {
      m_lastFrameAEPS = 0;

      ReinitUs();

      m_grid.Init();

      m_grid.InitThreads();

      // No longer needed?  Only needed in cpp-elt situations??  We shall see
      //      NeedElement(&Element_Empty<EC>::THE_INSTANCE);

      ReinitPhysics();

      ReinitEden();

      PostReinit(m_varguments);

      LoadFromConfigurationPath();

      m_grid.SetGridRunning(false);

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
         LOG.Message("Simulation driver exiting");
       });
    }

  protected:

    OurElementRegistry m_elementRegistry;

    Element<EC>* m_neededElements[MAX_NEEDED_ELEMENTS];
    u32 m_neededElementCount;

    OurStdElements m_se;
    OurGrid m_grid;

    u64 m_ticksLastStopped;
    u64 m_totalPriorTicks;
    u64 m_currentTickBasis;
    u32 m_haltAfterAEPS;
    bool m_haltOnExtinctionOf;
    u8 m_extinctionSymbol[3];
    bool m_haltOnEmpty;
    bool m_haltOnFull;
    bool m_suppressStdElements;
    bool m_includeUEDemos;
    bool m_includeCPPDemos;

    u64 m_msSpentRunning;
    u64 m_msSpentOverhead;
    s32 m_microsSleepPerFrame;
    double m_overheadPercent;
    double m_lastFrameAEPS;
    u32 m_aepsPerFrame;

    s32 m_AEPSPerEpoch;
    u32 m_autosavePerEpochs;
    u32 m_accelerateAfterEpochs;
    u32 m_acceleration;
    u32 m_surgeAfterEpochs;
    u32 m_maxEpochLength;

    bool m_gridImages;
    bool m_tileImages;

    double m_AEPS;

    /**
     * The absolute event rate since the beginning of the simulation
     */
    double m_AER;

    /**
     * The recent event rate computed by backwards averaging
     */
    double m_recentAER;

    /**
     * The previous value of Grid::GetTotalEventsExecuted, for
     * computing m_recentAET
     */
    u64 m_lastTotalEvents;

    u32 m_nextEpochAEPS;
    u32 m_epochCount;

    VArguments m_varguments;
    OString1024 m_commandLineArguments;

    u32 m_configurationPathCount;
    u32 m_currentConfigurationPath;
    const char* (m_configurationPaths[MAX_CONFIGURATION_PATHS]);

    char m_simDirBasePath[MAX_PATH_LENGTH];
    u32 m_simDirBasePathLength;

    ExternalConfig<GC> m_externalConfig;
    ExternalConfigSectionDriver<GC> m_externalConfigSectionDriver;
    ExternalConfigSectionGrid<GC> m_externalConfigSectionGrid;

  public:
    bool IsLoadDriverSection() const { return m_externalConfigSectionDriver.IsEnabled(); }
    void SetLoadDriverSection(bool val) { m_externalConfigSectionDriver.SetEnabled(val); }

    bool IsLoadGridSection() const { return m_externalConfigSectionGrid.IsEnabled(); }
    void SetLoadGridSection(bool val) { m_externalConfigSectionGrid.SetEnabled(val); }

  };
}

#endif /* ABSTRACTDRIVER_H */
