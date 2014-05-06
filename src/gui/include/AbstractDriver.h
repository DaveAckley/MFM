#ifndef ABSTRACTDRIVER_H   /* -*- C++ -*- */
#define ABSTRACTDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "AbstractButton.h"
#include "itype.h"
#include "Tile.h"
#include "Grid.h"
#include "GridRenderer.h"
#include "StatsRenderer.h"
#include "ElementTable.h"
#include "Element_Empty.h" /* Need common elements */
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "Element_Wall.h"
#include "Element_Consumer.h"
#include "Mouse.h"
#include "Keyboard.h"
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

#define STATS_WINDOW_WIDTH 288

#define STATS_START_WINDOW_WIDTH 233
#define STATS_START_WINDOW_HEIGHT 120

  /* super speedy for now */
#define MINIMAL_START_WINDOW_WIDTH 1
#define MINIMAL_START_WINDOW_HEIGHT 1


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

    Fonts m_fonts;

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
    Panel m_rootPanel;
    Drawing m_rootDrawing;

    u32 m_screenWidth;
    u32 m_screenHeight;

  protected: /* Need these for our buttons at driver level */
    GridRenderer m_grend;
    StatsRenderer<GC> m_srend;

  private:
    DriverArguments* m_driverArguments;

    void Sleep(u32 seconds, u64 nanos)
    {
      struct timespec tspec;
      tspec.tv_sec = seconds;
      tspec.tv_nsec = nanos;

      nanosleep(&tspec, NULL);
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
	  if(mouse.SemiAuto(SDL_BUTTON_LEFT))
	  {
	    m_srend.HandleClick(mloc);
	  }
	}

      }
      mouse.Flip();
    }

    inline void ToggleStatsView()
    {
      m_statisticsPanel.ToggleVisibility();
      m_grend.SetDimensions(Point<u32>(m_screenWidth - (renderStats ? STATS_WINDOW_WIDTH : 0)
				       , m_screenHeight));
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
	ToggleStatsView();
      }
      if(keyboard.SemiAuto(SDLK_1))
      {
	m_grend.IncreaseAtomSize();
      }
      if(keyboard.SemiAuto(SDLK_2))
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
      if(keyboard.IsDown(SDLK_COMMA))
      {
	if(m_aepsPerFrame > 1)
	  m_aepsPerFrame--;
      }
      if(keyboard.IsDown(SDLK_PERIOD))
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

	m_statisticsPanel.SetAEPS(m_AEPS = grid.GetTotalEventsExecuted() / grid.GetTotalSites());
	m_AER = 1000 * (m_AEPS / m_msSpentRunning);

	m_overheadPercent = 100.0*m_msSpentOverhead/(m_msSpentRunning+m_msSpentOverhead);

	double diff = m_AEPS - m_lastFrameAEPS;
	double err = MIN(1.0, MAX(-1.0, m_aepsPerFrame - diff));

	// Correct up to 20% of current each frame
	m_microsSleepPerFrame = (100+20*err)*m_microsSleepPerFrame/100;
	m_microsSleepPerFrame = MIN(100000000, MAX(1000, m_microsSleepPerFrame));

	m_lastFrameAEPS = m_AEPS;

	ExportEventCounts(grid);
        //	ExportTimeBasedData(grid);
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

	  path = GetSimDirPathTemporary("teps/%010d-average.ppm", m_nextEventCountsAEPS);
	  fp = fopen(path, "w");
	  grid.WriteEPSAverageImage(fp);
	  fclose(fp);

	  m_nextEventCountsAEPS += m_recordEventCountsPerAEPS;
	}
      }
    }

#if 0
    void ExportTimeBasedData(OurGrid& grid)
    {
      /* Current header : */
      /* # AEPS activesites empty dreg res wall sort-hits sort-misses sort-total sort-hit-pctg sort-bucket-miss-average*/

      if(m_recordTimeBasedDataPerAEPS > 0)
      {
	if(m_AEPS > m_nextTimeBasedDataAEPS)
	{
	  const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
	  FILE* fp = fopen(path, "a");

	  u64 consumed = 0, totalError = 0;
          for (typename OurGrid::iterator_type i = grid.begin(); i != grid.end(); ++i) {
            Tile<CC> * t = *i;
            consumed += Element_Consumer<CC>::THE_INSTANCE.GetAndResetDatumsConsumed(*t);
            totalError += Element_Consumer<CC>::THE_INSTANCE.GetAndResetBucketError(*t);
          }

	  fprintf(fp, "%g %d %d %d %d %d %ld %ld\n",
		  m_AEPS,
		  grid.CountActiveSites(),
		  grid.GetAtomCount(Element_Empty<CC>::TYPE),
		  grid.GetAtomCount(Element_Dreg<CC>::TYPE),
		  grid.GetAtomCount(Element_Res<CC>::TYPE),
		  grid.GetAtomCount(Element_Wall<CC>::TYPE),
		  consumed, totalError);

	  fclose(fp);
	  m_nextTimeBasedDataAEPS += m_recordTimeBasedDataPerAEPS;
	}
      }
    }
#endif

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
      u64 startTime = Utils::GetDateTimeNow();

      /* Get the master simulation data directory */
      snprintf(m_simDirBasePath, MAX_PATH_LENGTH-1,
	       "%s/%ld/", dirPath, startTime);

      m_simDirBasePathLength = strlen(m_simDirBasePath);
      if (m_simDirBasePathLength >= MAX_PATH_LENGTH-MIN_PATH_RESERVED_LENGTH)
        args.Die("Path name too long '%s'",dirPath);

      /* Make the std subdirs under it */
      const char * (subs[]) = { "", "vid", "eps", "tbd", "teps" };
      for (u32 i = 0; i < sizeof(subs)/sizeof(subs[0]); ++i) {
        const char * path = GetSimDirPathTemporary("%s", subs[i]);
        if (mkdir(path, 0777) != 0)
          args.Die("Couldn't make simulation sub directory '%s': %s",path,strerror(errno));
      }

      /* Initialize tbd.txt */
      const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
      FILE* fp = fopen(path, "w");
      fprintf(fp, "# AEPS activesites empty dreg res wall sort-hits sort-misses sort-total sort-hit-pctg\n");
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

      SDL_Init(SDL_INIT_VIDEO);
      m_fonts.Init();

      m_rootPanel.SetName("Root");
      m_gridPanel.SetGridRenderer(&m_grend);
      m_gridPanel.SetGrid(&mainGrid);

      m_statisticsPanel.SetStatsRenderer(&m_srend);
      m_statisticsPanel.SetGrid(&mainGrid);
      m_statisticsPanel.SetAEPS(m_AEPS);
      m_statisticsPanel.SetAER(m_AER);
      m_statisticsPanel.SetAEPSPerFrame(m_aepsPerFrame);
      m_statisticsPanel.SetOverheadPercent(m_overheadPercent);
      m_statisticsPanel.SetVisibility(false);

      m_rootPanel.Insert(&m_gridPanel, NULL);
      m_gridPanel.Insert(&m_statisticsPanel, NULL);
      /*
      m_rootPanel.Insert(&m_panel1,0);
      m_rootPanel.Insert(&m_panel2,&m_panel1);
      m_panel1.Insert(&m_panel3,0);
      m_rootPanel.Insert(&m_panel4,0);
      m_panel5.SetControlled(&m_panel4);
      m_rootPanel.Insert(&m_panel5,0);
      */
      m_rootPanel.Print(stdout);

      m_srend.OnceOnly(m_fonts);

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

      HandleResize();

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

      renderStats = false;
      if(args.GetStartWithoutGridView())
      {
	m_screenWidth = STATS_START_WINDOW_WIDTH;
	m_screenHeight = STATS_START_WINDOW_HEIGHT;
	ToggleStatsView();
	m_srend.SetDisplayAER(!m_srend.GetDisplayAER());
      }
      else if(args.GetStartMinimal())
      {
	m_screenWidth = MINIMAL_START_WINDOW_HEIGHT;
	m_screenHeight = MINIMAL_START_WINDOW_WIDTH;
      }

      m_aepsPerFrame = args.GetAEPSPerFrame();
    }

    /**
     * Register any element types needed for the run
     */
    virtual void ReinitPhysics() = 0;

    /**
     * Establish the Garden of Eden configuration on the grid
     */
    virtual void ReinitEden() = 0;

    virtual void HandleResize() = 0;

    OurGrid & GetGrid()
    {
      return mainGrid;
    }

    void ToggleTileView()
    {
      m_grend.ToggleMemDraw();
    }

    //////////////////
    //////// START DEBUG PANELS

    struct Panel1 : public Panel {
      Panel1() {
        SetName("Panel1");
        SetDimensions(400,500);
        SetRenderPoint(SPoint(280,43));
        SetForeground(Drawing::BLUE);
        SetBackground(Drawing::GREEN);
      }
    } m_panel1;

    class GridPanel : public Panel
    {
      GridRenderer* m_grend;
      OurGrid* m_mainGrid;

    public:
      GridPanel()
      {
	SetName("Grid Panel");
	SetDimensions(SCREEN_INITIAL_WIDTH,
		      SCREEN_INITIAL_HEIGHT);
	SetRenderPoint(SPoint(0, 0));
	SetForeground(Drawing::BLACK);
	SetBackground(Drawing::DARK_PURPLE);

	m_grend = NULL;
	m_mainGrid = NULL;
      }

      void SetGrid(OurGrid* mainGrid)
      {
	m_mainGrid = mainGrid;
      }

      void SetGridRenderer(GridRenderer* grend)
      {
	m_grend = grend;
      }

    protected:
      virtual void PaintComponent(Drawing& drawing)
      {
	this->Panel::PaintComponent(drawing);

	m_grend->RenderGrid(drawing, *m_mainGrid);
      }

      virtual bool Handle(SDL_MouseButtonEvent& event)
      {
	if(event.button == SDL_BUTTON_LEFT)
	{
	  SPoint pt = GetAbsoluteLocation();
	  pt.Set(event.x - pt.GetX(),
		 event.y - pt.GetY());

	  m_grend->SelectTile(*m_mainGrid,
			      pt);
	}
	return true;
      }
    } m_gridPanel;

    class StatisticsPanel : public Panel
    {
      StatsRenderer<GC>* m_srend;
      OurGrid* m_mainGrid;
      double m_AEPS;
      double m_AER;
      double m_overheadPercent;
      u32 m_aepsPerFrame;

    public:
      StatisticsPanel() : m_srend(NULL)
      {
	SetName("Statistics Panel");
	SetDimensions(STATS_START_WINDOW_WIDTH,
		      SCREEN_INITIAL_HEIGHT);
	SetRenderPoint(SPoint(SCREEN_INITIAL_WIDTH - STATS_START_WINDOW_WIDTH, 0));
	SetForeground(Drawing::WHITE);
	SetBackground(Drawing::DARK_PURPLE);
	m_AEPS = m_AER = 0.0;
	m_aepsPerFrame = 0;
      }

      void SetStatsRenderer(StatsRenderer<GC>* srend)
      {
	m_srend = srend;
      }

      void SetGrid(OurGrid* mainGrid)
      {
	m_mainGrid = mainGrid;
      }

      void SetAEPS(double aeps)
      {
	m_AEPS = aeps;
      }

      void SetAER(double aer)
      {
	m_AER = aer;
      }

      void SetOverheadPercent(double overheadPercent)
      {
	m_overheadPercent = overheadPercent;
      }

      void SetAEPSPerFrame(u32 apf)
      {
	m_aepsPerFrame = apf;
      }

    protected:
      virtual void PaintComponent(Drawing& drawing)
      {
	this->Panel::PaintComponent(drawing);
	m_srend->RenderGridStatistics(drawing, *m_mainGrid,
				     m_AEPS, m_AER, m_aepsPerFrame,
				     m_overheadPercent, false);
      }

      virtual bool Handle(SDL_MouseButtonEvent& event)
      {
	if(event.button == SDL_BUTTON_LEFT)
	{
	  SPoint pt = GetAbsoluteLocation();
	  pt.Set(event.x - pt.GetX(),
		 event.y - pt.GetY());
	  printf("(%d,%d)\n", pt.GetX(), pt.GetY());
	}
	return true;
      }
    }m_statisticsPanel;

    struct Panel2 : public Panel {
      Panel2() {
        SetName("Panel2");
        SetDimensions(20,20);
        SetRenderPoint(SPoint(0,0));
        SetForeground(Drawing::BLUE);
        SetBackground(Drawing::BLACK);
      }
      u32 m_counter;
      virtual void PaintComponent(Drawing & drawing) {
        ++m_counter;
        SetRenderPoint(SPoint(m_counter%20+30, m_counter%30+40));
        this->Panel::PaintComponent(drawing);
      }
    } m_panel2;

    struct Panel3 : public Panel {
      Panel3() {
        SetName("Panel3");
        SetDimensions(20,20);
        SetRenderPoint(SPoint(0,0));
        SetForeground(Drawing::CYAN);
        SetBackground(Drawing::MAGENTA);
      }
      u32 m_counter;
      virtual void PaintComponent(Drawing & drawing) {
        m_counter += 2;
        SetRenderPoint(SPoint(m_counter%50+30, m_counter%80+40));
        this->Panel::PaintComponent(drawing);
      }
    } m_panel3;

    struct Panel4 : public AbstractButton {
      Panel4() : AbstractButton("Panel4") {
        SetName("Panel4");
        Panel::SetDimensions(100,40);
        SetRenderPoint(SPoint(50,70));
        SetForeground(Drawing::BLACK);
        SetBackground(Drawing::WHITE);
      }
      virtual void OnClick() {
        printf("click\n");
      }
    } m_panel4;

    struct Panel5 : public AbstractButton {
      AbstractButton * m_toEnable;
      Panel5() : AbstractButton("Enabler") {
        SetName("Enable");
        Panel::SetDimensions(100,40);
        SetRenderPoint(SPoint(50,200));
        SetForeground(Drawing::BLACK);
        SetBackground(Drawing::WHITE);
      }
      void SetControlled(AbstractButton * controlled) {
        m_toEnable = controlled;
      }
      virtual void OnClick() {
        m_toEnable->SetEnabled(!m_toEnable->IsEnabled());
      }
    } m_panel5;

    //////// END DEBUG PANELS
    //////////////////


    void SetScreenSize(u32 width, u32 height) {
      m_screenWidth = width;
      m_screenHeight = height;
      screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);
      if (screen == 0)
        FAIL(ILLEGAL_STATE);

      m_rootPanel.SetDimensions(m_screenWidth, m_screenHeight);
      m_rootPanel.SetRenderPoint(SPoint(0, 0));
      m_rootPanel.SetForeground(Drawing::BLUE);
      m_rootPanel.SetBackground(Drawing::RED);

      m_rootDrawing.Reset(screen, m_fonts.GetDefaultFont());

      if(renderStats)
      {
	m_grend.SetDimensions(UPoint(m_screenWidth - STATS_WINDOW_WIDTH,m_screenHeight));
      }
      else
      {
	m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));
      }

      //m_srend.SetDestination(screen);
      m_srend.SetDrawPoint(SPoint(0,0));//m_screenWidth-STATS_WINDOW_WIDTH, 0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));

      printf("Screen resize: %d x %d\n", width, height);
      HandleResize();
    }

    StatsRenderer<GC> & GetStatsRenderer()
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
      SetScreenSize(m_screenWidth, m_screenHeight);

      /*
      screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);
      if (screen == 0)
        FAIL(ILLEGAL_STATE);


      //      m_grend.SetDestination(screen);
      m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));

      //m_srend.SetDestination(screen);
      m_srend.SetDrawPoint(SPoint(m_screenWidth-STATS_WINDOW_WIDTH, 0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));
      */

      SDL_Event event;
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
                  //                  mouse.HandleButtonEvent(&event.button);
                  //                  break;
                case SDL_MOUSEMOTION:
                  m_rootPanel.Dispatch(event,Rect(SPoint(),UPoint(m_screenWidth,m_screenHeight)));
                  //                  mouse.HandleMotionEvent(&event.motion);
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

          //m_rootDrawing.SetBackground(0xff200020);
          m_rootDrawing.Clear();

          m_rootPanel.Paint(m_rootDrawing);
          //          m_grend.RenderGrid(m_rootDrawing, mainGrid);

          if(renderStats)
          {
	  }

          if (m_recordScreenshotPerAEPS > 0) {
            if (!paused && m_AEPS >= m_nextScreenshotAEPS) {

              const char * path = GetSimDirPathTemporary("vid/%010d.png", m_nextScreenshotAEPS);

              camera.DrawSurface(screen,path);
              {
                const char * path = GetSimDirPathTemporary("tbd/data.dat");
                bool exists = true;
                {
                  FILE* fp = fopen(path, "r");
                  if (!fp) exists = false;
                  else fclose(fp);
                }
                FILE* fp = fopen(path, "a");
                m_srend.WriteRegisteredCounts(fp, !exists, mainGrid,
                                              m_AEPS, m_AER, m_aepsPerFrame, m_overheadPercent, true);
                fclose(fp);
              }
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
