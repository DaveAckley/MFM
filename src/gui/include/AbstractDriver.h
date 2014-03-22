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

#define EVENTS_PER_FRAME 100000

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024

#define STATS_WINDOW_WIDTH 256

#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

  template<class ATOM, u32 WIDTH, u32 HEIGHT, u32 RADIUS>
  class AbstractDriver
  {
  public:
    static const u32 EVENT_WINDOW_RADIUS = RADIUS;
    static const u32 GRID_WIDTH = WIDTH;
    static const u32 GRID_HEIGHT = HEIGHT;
    static const u32 ELEMENT_TABLE_BITS = 8;

    typedef Grid<ATOM,EVENT_WINDOW_RADIUS,GRID_WIDTH,GRID_HEIGHT> OurGrid;
    typedef ElementTable<ATOM,EVENT_WINDOW_RADIUS,ELEMENT_TABLE_BITS> OurElementTable;

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

    u32 m_eventsPerFrame;
    double m_AER;
    double m_AEPS;
    u64 m_msSpentRunning;

    s32 m_recordEventCountsPerAEPS;
    s32 m_recordScreenshotPerAEPS;
    u32 m_nextEventCountsAEPS;
    u32 m_nextScreenshotAEPS;

    Mouse mouse;
    Keyboard keyboard;
    Camera camera;
    SDL_Surface* screen;

    u32 m_screenWidth;
    u32 m_screenHeight;

    GridRenderer m_grend;
    StatsRenderer m_srend;

    void Sleep(u32 seconds, u64 nanos)
    {
      struct timespec tspec;
      tspec.tv_sec = seconds;
      tspec.tv_nsec = nanos;

      nanosleep(&tspec, NULL);
    }

    void Update(OurGrid& grid)
    {
      u8 speed = keyboard.ShiftHeld() ?
        CAMERA_FAST_SPEED : CAMERA_SLOW_SPEED;

      if(keyboard.IsDown(SDLK_q) && (keyboard.IsDown(SDLK_LCTRL) || keyboard.IsDown(SDLK_RCTRL)))
        {
          exit(0);
        }

      if(keyboard.IsDown(SDLK_a))
        {
          m_srend.SetDisplayAER(!m_srend.GetDisplayAER());
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
      if(keyboard.SemiAuto(SDLK_r))
        {
          camera.ToggleRecord();
        }
      if(keyboard.SemiAuto(SDLK_e))
        {
          grid.TriggerEvent();
        }
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
      if(keyboard.SemiAuto(SDLK_SPACE))
        {
          paused = !paused;
        }
      if(keyboard.SemiAuto(SDLK_i))
        {
          renderStats = !renderStats;
          if(!renderStats)
            {
              m_grend.SetDimensions(Point<u32>(m_screenWidth, m_screenHeight));
            }
          else
            {
              m_grend.SetDimensions(Point<u32>(m_screenWidth - STATS_WINDOW_WIDTH,
                                               m_screenHeight));
            }
        }
      if(keyboard.IsDown(SDLK_RIGHT))
        {
          m_grend.MoveRight(speed);
        }
      if(keyboard.SemiAuto(SDLK_COMMA))
        {
          m_eventsPerFrame /= 10;
          if(m_eventsPerFrame == 0)
            {
              m_eventsPerFrame = 1;
            }
        }
      if(keyboard.SemiAuto(SDLK_PERIOD))
        {
          m_eventsPerFrame *= 10;
        }

      if(!paused)
        {
          u32 startMS = SDL_GetTicks();
          grid.Unpause();
          Sleep(2, 100000000);
          grid.Pause();
          m_msSpentRunning += (SDL_GetTicks() - startMS);

          m_AEPS = grid.GetTotalEventsExecuted() / grid.GetTotalSites();
          m_AER = 1000 * (m_AEPS / m_msSpentRunning);

          if (m_recordEventCountsPerAEPS > 0) {
            if (m_AEPS >= m_nextEventCountsAEPS) {

              char buf[100];
              snprintf(buf,100,"%10d-EPS.ppm",m_nextEventCountsAEPS);
              FILE* fp = fopen(buf, "w");
              grid.WriteEPSImage(fp);
              fclose(fp);

              m_nextEventCountsAEPS += m_recordEventCountsPerAEPS;
            }
          }
        }

      mouse.Flip();
      keyboard.Flip();
    }

  public:

    AbstractDriver(DriverArguments & args)
    {
      OnceOnly(args);
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

      /* Sim directory = now */
      u64 startTime = GetDateTimeNow();

      /* Get the master simulation data directory */
      snprintf(m_simDirBasePath, MAX_PATH_LENGTH-1,
	       "%s/%ld/", dirPath, startTime);

      m_simDirBasePathLength = strlen(m_simDirBasePath);
      if (m_simDirBasePathLength >= MAX_PATH_LENGTH-MIN_PATH_RESERVED_LENGTH)
        args.Die("Path name too long '%s'",dirPath);

      /* Make the std subdirs under it */
      const char * (subs[]) = { "", "vid", "eps" };
      for (u32 i = 0; i < sizeof(subs)/sizeof(subs[0]); ++i) {
        const char * path = GetSimDirPathTemporary("%s", subs[i]);
        if (mkdir(path, 0777) != 0)
          args.Die("Couldn't make simulation sub directory '%s': %s",path,strerror(errno));
      }

      m_screenWidth = SCREEN_INITIAL_WIDTH;
      m_screenHeight = SCREEN_INITIAL_HEIGHT;

      m_eventsPerFrame = EVENTS_PER_FRAME;
      m_AEPS = 0;
      m_msSpentRunning = 0;

      m_recordEventCountsPerAEPS = args.GetRecordEventCountsPerAEPS();
      m_recordScreenshotPerAEPS = args.GetRecordScreenshotPerAEPS();

      u32 seed = args.GetSeed();
      if (seed==0) seed = time(0);
      SetSeed(seed);

      SDL_Init(SDL_INIT_EVERYTHING);
      TTF_Init();

      m_srend.OnceOnly();

      SDL_WM_SetCaption("Movable Feast Machine Simulator", NULL);
    }

    void SetSeed(u32 seed) {
      if (seed == 0)
        FAIL(ILLEGAL_ARGUMENT);
      mainGrid.SetSeed(seed);
    }

    void ReinitUs() {
      m_nextEventCountsAEPS = 0;
      m_nextScreenshotAEPS = 0;
    }

    void Reinit() {
      ReinitUs();

      mainGrid.Reinit();

      mainGrid.Needed(Element_Empty<P1Atom, 4>::THE_INSTANCE);

      ReinitPhysics();

      ReinitEden();
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
      paused = true;

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
              m_srend.RenderGridStatistics(mainGrid, m_AEPS, m_AER);
            }

          if (m_recordScreenshotPerAEPS > 0) {
            if (m_AEPS >= m_nextScreenshotAEPS) {

              const char * path = GetSimDirPathTemporary("vid/%010d.png", m_nextScreenshotAEPS);

              camera.DrawSurface(screen,path);

              m_nextScreenshotAEPS += m_recordScreenshotPerAEPS;
            }
          }
          SDL_Flip(screen);
        }

      SDL_FreeSurface(screen);
      SDL_Quit();
    }
  };
} /* namespace MFM */

#endif /* ABSTRACTDRIVER_H */
