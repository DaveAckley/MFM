#ifndef ABSTRACTDRIVER_H   /* -*- C++ -*- */
#define ABSTRACTDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "itype.h"
#include "grid.h"
#include "gridrenderer.h"
#include "statsrenderer.h"
#include "ElementTable.h"
#include "mouse.h"
#include "keyboard.h"
#include "Camera.h"
#include "DriverArguments.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

namespace MFM {

#define FRAMES_PER_SECOND 100.0

#define INITIAL_AEPS_PER_FRAME 1

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024

#define STATS_WINDOW_WIDTH 256

#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

  template<class GC>
  class AbstractDriver
  {
  public:
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
    char m_simDirBasePath[MAX_PATH_LENGTH];
    u32 m_simDirBasePathLength;

    char * GetSimDirPathTemporary(const char * format, ...) {
      va_list ap;
      va_start(ap,format);
      vsnprintf(m_simDirBasePath+m_simDirBasePathLength, MAX_PATH_LENGTH-1, format, ap);
      return m_simDirBasePath;
    }

    OurGrid mainGrid;

    bool paused;

    bool renderStats;

    bool m_startPaused;
    u32 m_haltAfterAEPS;
    u32 m_aepsPerFrame;
    s32 m_microsSleepPerFrame;
    double m_overheadPercent;
    double m_AER;
    double m_AEPS;
    double m_lastFrameAEPS;
    u64 m_msSpentRunning;
    u64 m_msSpentOverhead;
    u32 m_ticksLastStopped;

    s32 m_recordEventCountsPerAEPS;
    s32 m_recordScreenshotPerAEPS;
    s32 m_recordTimeBasedDataPerAEPS;
    s32 m_maxRecordScreenshotPerAEPS;
    s32 m_countOfScreenshotsAtThisAEPS;
    s32 m_countOfScreenshotsPerRate;
    u32 m_nextEventCountsAEPS;
    u32 m_nextScreenshotAEPS;
    u32 m_nextTimeBasedDataAEPS;

    Mouse mouse;
    Keyboard keyboard;
    Camera camera;
    SDL_Surface* screen;

    u32 m_screenWidth;
    u32 m_screenHeight;

    GridRenderer m_grend;
    StatsRenderer m_srend;

    DriverArguments* m_driverArguments;

    void Sleep(u32 seconds, u64 nanos)
    {
      struct timespec tspec;
      tspec.tv_sec = seconds;
      tspec.tv_nsec = nanos;

      nanosleep(&tspec, NULL);
    }

    typedef enum
    {
      BUTTONFUNC_TOGGLE_EXECUTION,
      BUTTONFUNC_EMPTY_TILE
    }ButtonFunction;

    void ExecuteButtonFunction(ButtonFunction func)
    {
      switch(func)
      {
      case BUTTONFUNC_TOGGLE_EXECUTION:
	mainGrid.SetTileToExecuteOnly(m_grend.GetSelectedTile(),
				      !mainGrid.GetTileExecutionStatus(m_grend.GetSelectedTile()));
	break;
      case BUTTONFUNC_EMPTY_TILE:
	mainGrid.EmptyTile(m_grend.GetSelectedTile());
	break;
      default:
	FAIL(ILLEGAL_ARGUMENT);
	break;
      }
    }

    void Update(OurGrid& grid)
    {
      KeyboardUpdate(grid);
      MouseUpdate(grid);
      RunGrid(grid);
    }

    void MouseUpdate(OurGrid& grid)
    {
      if(mouse.IsDown(SDL_BUTTON_LEFT))
      {
	SPoint mloc;
	UPoint gdim = m_grend.GetDimensions();

	mouse.FillPoint(&mloc);

	if((u32)mloc.GetX() < gdim.GetX() &&
	   (u32)mloc.GetY() < gdim.GetY())
	{
	  /* This is in the grid renderer */
	  m_grend.SelectTile(grid, mloc);
	}
	else
	{
	  /* This must be in the control panel  */

	  /* Adjust so we are relative to the control panel */
	  mloc.SetX(mloc.GetX() - (s32)gdim.GetX());
	}

      }
      mouse.Flip();
    }

    void KeyboardUpdate(OurGrid& grid)
    {
      u8 speed = keyboard.ShiftHeld() ?
        CAMERA_FAST_SPEED : CAMERA_SLOW_SPEED;

      if(keyboard.IsDown(SDLK_q) && (keyboard.IsDown(SDLK_LCTRL) || keyboard.IsDown(SDLK_RCTRL)))
      {
	exit(0);
      }
      
      /* View Control */
      if(keyboard.SemiAuto(SDLK_a))
      {
	m_srend.SetDisplayAER(!m_srend.GetDisplayAER());
      }
      if(keyboard.SemiAuto(SDLK_i))
      {
	renderStats = !renderStats;
	  m_grend.SetDimensions(Point<u32>(m_screenWidth - (renderStats ? STATS_WINDOW_WIDTH : 0)
					   , m_screenHeight));
      }
      if(keyboard.IsDown(SDLK_1))
      {
	m_grend.IncreaseAtomSize();
      }
      if(keyboard.IsDown(SDLK_2))
      {
	m_grend.DecreaseAtomSize();
      }
      if(keyboard.SemiAuto(SDLK_g))
      {
	m_grend.ToggleGrid();
      }
      if(keyboard.SemiAuto(SDLK_m))
      {
	m_grend.ToggleMemDraw();
      }
      if(keyboard.SemiAuto(SDLK_l))
      {
	m_grend.ToggleDataHeatmap();
      }
      if(keyboard.SemiAuto(SDLK_p))
      {
	m_grend.ToggleTileSeparation();
      }
      if(keyboard.SemiAuto(SDLK_o))
      {
	m_grend.SetEventWindowRenderMode(EVENTWINDOW_RENDER_OFF);
      }

      /* Per-Tile Control */
      if(keyboard.SemiAuto(SDLK_9))
      {
        ExecuteButtonFunction(BUTTONFUNC_TOGGLE_EXECUTION);
      }
      if(keyboard.SemiAuto(SDLK_8))
      {
	ExecuteButtonFunction(BUTTONFUNC_EMPTY_TILE);
      }
      if(keyboard.SemiAuto(SDLK_ESCAPE))
      {
	m_grend.DeselectTile();
      }

      /* Camera Recording */
      if(keyboard.SemiAuto(SDLK_r))
      {
	camera.ToggleRecord();
      }

      /* Camera Movement*/
      if(keyboard.IsDown(SDLK_LEFT))
      {
	m_grend.MoveLeft(speed);
      }
      if(keyboard.IsDown(SDLK_DOWN))
      {
	m_grend.MoveDown(speed);
      }
      if(keyboard.IsDown(SDLK_UP))
      {
	m_grend.MoveUp(speed);
      }
      if(keyboard.IsDown(SDLK_RIGHT))
      {
	m_grend.MoveRight(speed);
      }

      /* Speed Control */
      if(keyboard.SemiAuto(SDLK_SPACE))
      {
	paused = !paused;
      }
      if(keyboard.SemiAuto(SDLK_COMMA))
      {
	if(m_aepsPerFrame > 1)
	  m_aepsPerFrame--;
      }
      if(keyboard.SemiAuto(SDLK_PERIOD))
      {
	if(m_aepsPerFrame < 1000)
	  m_aepsPerFrame++;
      }

      keyboard.Flip();
    }

    void RunGrid(OurGrid& grid)
    {
      if(!paused)
      {
	const s32 ONE_THOUSAND = 1000;
	const s32 ONE_MILLION = ONE_THOUSAND*ONE_THOUSAND;

	grid.Unpause();  // pausing and unpausing should be overhead!

	u32 startMS = SDL_GetTicks();  // So get the ticks after unpausing
	if (m_ticksLastStopped != 0)
	  m_msSpentOverhead += startMS - m_ticksLastStopped;
	else
	  m_msSpentOverhead = 0;

	Sleep(m_microsSleepPerFrame/ONE_MILLION, (u64) (m_microsSleepPerFrame%ONE_MILLION)*ONE_THOUSAND);
	m_ticksLastStopped = SDL_GetTicks(); // and before pausing

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
	ExportTimeBasedData(grid);
      }
    }

    void ExportEventCounts(OurGrid& grid)
    {
      if (m_recordEventCountsPerAEPS > 0) {
	if (m_AEPS > m_nextEventCountsAEPS) {
	  
	  const char * path = GetSimDirPathTemporary("eps/%010d.ppm", m_nextEventCountsAEPS);
	  FILE* fp = fopen(path, "w");
	  grid.WriteEPSImage(fp);
	  fclose(fp);
	  
	  m_nextEventCountsAEPS += m_recordEventCountsPerAEPS;
	}
      }
    }
    
    void ExportTimeBasedData(OurGrid& grid)
    {
      /* Current header : */
      /* # AEPS activesites empty dreg res wall  */

      if(m_recordTimeBasedDataPerAEPS > 0)
      {
	if(m_AEPS > m_nextTimeBasedDataAEPS)
	{
	  const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
	  FILE* fp = fopen(path, "a");
	  
	  fprintf(fp, "%g %d\n", m_AEPS, grid.CountActiveSites());
	  
	  fclose(fp);
	  m_nextTimeBasedDataAEPS += m_recordTimeBasedDataPerAEPS;
	}
      }
    }

  public:

    AbstractDriver(DriverArguments & args)
    {
      OnceOnly(args);

      /* Let's save these for later */
      m_driverArguments = &args;
    }

    void OnceOnly(DriverArguments & args)
    {
      const char * dirPath = args.GetDataDirPath();
      if (dirPath == 0) dirPath = "/tmp";

      /* Try to make the main dir */
      if (mkdir(dirPath,0777) != 0) {
        /* If it died because it's already there we'll let it ride.. */
        if (errno != EEXIST)
          args.Die("Couldn't make directory '%s': %s",dirPath,strerror(errno));
      }

      m_startPaused = args.GetStartPaused();
      m_haltAfterAEPS = args.GetHaltAfterAEPS();

      /* Sim directory = now */
      u64 startTime = GetDateTimeNow();

      /* Get the master simulation data directory */
      snprintf(m_simDirBasePath, MAX_PATH_LENGTH-1,
	       "%s/%ld/", dirPath, startTime);

      m_simDirBasePathLength = strlen(m_simDirBasePath);
      if (m_simDirBasePathLength >= MAX_PATH_LENGTH-MIN_PATH_RESERVED_LENGTH)
        args.Die("Path name too long '%s'",dirPath);

      /* Make the std subdirs under it */
      const char * (subs[]) = { "", "vid", "eps", "tbd" };
      for (u32 i = 0; i < sizeof(subs)/sizeof(subs[0]); ++i) {
        const char * path = GetSimDirPathTemporary("%s", subs[i]);
        if (mkdir(path, 0777) != 0)
          args.Die("Couldn't make simulation sub directory '%s': %s",path,strerror(errno));
      }

      /* Initialize tbd.txt */
      const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
      FILE* fp = fopen(path, "w");
      fprintf(fp, "# AEPS activesites empty dreg res wall\n");
      fclose(fp);

      m_screenWidth = SCREEN_INITIAL_WIDTH;
      m_screenHeight = SCREEN_INITIAL_HEIGHT;

      m_aepsPerFrame = INITIAL_AEPS_PER_FRAME;
      m_AEPS = 0;
      m_msSpentRunning = 0;
      m_microsSleepPerFrame = 1000;

      m_recordEventCountsPerAEPS = args.GetRecordEventCountsPerAEPS();
      m_recordScreenshotPerAEPS = args.GetRecordScreenshotPerAEPS();
      m_countOfScreenshotsPerRate = args.GetCountOfScreenshotsPerRate();
      m_recordTimeBasedDataPerAEPS = args.GetRecordTimeBasedDataPerAEPS();
      if (m_countOfScreenshotsPerRate > 0) {
        m_maxRecordScreenshotPerAEPS = m_recordScreenshotPerAEPS;
        m_recordScreenshotPerAEPS = 1;
        m_countOfScreenshotsAtThisAEPS = 0;
      }

      u32 seed = args.GetSeed();
      if (seed==0) seed = time(0);
      SetSeed(seed);

      SDL_Init(SDL_INIT_EVERYTHING);
      TTF_Init();

      m_srend.OnceOnly();

      SDL_WM_SetCaption("Movable Feast Machine Simulator", NULL);

      m_ticksLastStopped = 0;
    }

    void SetSeed(u32 seed) {
      if (seed == 0)
        FAIL(ILLEGAL_ARGUMENT);
      mainGrid.SetSeed(seed);
    }

    void ReinitUs() {
      m_nextEventCountsAEPS = 0;
      m_nextScreenshotAEPS = 0;
      m_nextTimeBasedDataAEPS = 0;
      m_lastFrameAEPS = 0;
    }

    void Reinit() {
      ReinitUs();

      mainGrid.Reinit();

      mainGrid.Needed(Element_Empty<CC>::THE_INSTANCE);

      ReinitPhysics();

      ReinitEden();

      ReapplyPostArguments(m_driverArguments);
    }

    void ReapplyPostArguments(DriverArguments* argptr)
    {
      DriverArguments& args = *argptr;

      /* Initially disable all tiles given in arguments */
      for(u32 i = 0; i < args.GetDisabledTileCount(); i++)
      {
	SPoint& pt =  args.GetDisabledTiles()[i];
	mainGrid.SetTileToExecuteOnly(pt, false);
      }
    }

    /**
     * Register any element types needed for the run
     */
    virtual void ReinitPhysics() = 0;

    /**
     * Establish the Garden of Eden configuration on the grid
     */
    virtual void ReinitEden() = 0;

    OurGrid & GetGrid()
    {
      return mainGrid;
    }

    void SetScreenSize(u32 width, u32 height) {
      m_screenWidth = width;
      m_screenHeight = height;
      screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);
      if (screen == 0)
        FAIL(ILLEGAL_STATE);

      m_grend.SetDestination(screen);
      m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));

      m_srend.SetDestination(screen);
      m_srend.SetDrawPoint(SPoint(m_screenWidth-STATS_WINDOW_WIDTH, 0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));
    }

    StatsRenderer & GetStatsRenderer()
    {
      return m_srend;
    }

    void Run()
    {
      unwind_protect({
          MFMPrintErrorEnvironment(stderr,&unwindProtect_errorEnvironment);
          fprintf(stderr, "Failure reached top-level!  Aborting\n");
          abort();
        }, {
          RunHelper();
        });
    }

    void RunHelper()
    {
      paused = m_startPaused;

      bool running = true;
      screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);
      if (screen == 0)
        FAIL(ILLEGAL_STATE);

      SDL_Event event;

      m_grend.SetDestination(screen);
      m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));

      m_srend.SetDestination(screen);
      m_srend.SetDrawPoint(SPoint(m_screenWidth-STATS_WINDOW_WIDTH, 0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));

      renderStats = false;

      s32 lastFrame = SDL_GetTicks();

      while(running)
        {
          while(SDL_PollEvent(&event))
            {
              switch(event.type)
                {
                case SDL_VIDEORESIZE:
                  SetScreenSize(event.resize.w, event.resize.h);
                  break;
                case SDL_QUIT:
                  running = false;
                  break;
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                  mouse.HandleButtonEvent(&event.button);
                  break;
                case SDL_MOUSEMOTION:
                  mouse.HandleMotionEvent(&event.motion);
                  break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                  keyboard.HandleEvent(&event.key);
                  break;

                }
            }

          /* Limit framerate */
          s32 sleepMS = (s32)
            ((1000.0 / FRAMES_PER_SECOND) -
             (SDL_GetTicks() - lastFrame));
          if(sleepMS > 0)
          {
	    SDL_Delay(sleepMS);
	  }
          lastFrame = SDL_GetTicks();


          Update(mainGrid);

          Drawing::Clear(screen, 0xff200020);

          m_grend.RenderGrid(mainGrid);
          if(renderStats)
          {
	    m_srend.RenderGridStatistics(mainGrid, m_AEPS, m_AER, m_aepsPerFrame, m_overheadPercent);
	  }

          if (m_recordScreenshotPerAEPS > 0) {
            if (!paused && m_AEPS >= m_nextScreenshotAEPS) {

              const char * path = GetSimDirPathTemporary("vid/%010d.png", m_nextScreenshotAEPS);

              camera.DrawSurface(screen,path);

              // Are we accelerating and not yet up to cruising speed?
              if (m_countOfScreenshotsPerRate > 0 && 
                  m_recordScreenshotPerAEPS < m_maxRecordScreenshotPerAEPS) {

                // Time to step on it?
                if (++m_countOfScreenshotsAtThisAEPS > m_countOfScreenshotsPerRate) {
                  ++m_recordScreenshotPerAEPS;
                  m_countOfScreenshotsAtThisAEPS = 0;
                }
              }
            
              m_nextScreenshotAEPS += m_recordScreenshotPerAEPS;
            }
          }

	if(m_haltAfterAEPS > 0 && m_AEPS > m_haltAfterAEPS)
        {
	  running = false;
	}
	
	SDL_Flip(screen);
      }
      
      SDL_FreeSurface(screen);
      SDL_Quit();
    }
  };
} /* namespace MFM */

#endif /* ABSTRACTDRIVER_H */
