#ifndef ABSTRACTGUIDRIVER_H   /* -*- C++ -*- */
#define ABSTRACTGUIDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "Logger.h"
#include "AbstractButton.h"
#include "Tile.h"
#include "GridRenderer.h"
#include "StatsRenderer.h"
#include "Element_Empty.h" /* Need common elements */
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "Element_Wall.h"
#include "Element_Consumer.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Camera.h"
#include "AbstractDriver.h"
#include "VArguments.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

namespace MFM {

#define FRAMES_PER_SECOND 100.0

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
  class AbstractGUIDriver : public AbstractDriver<GC>
  {
  private:
    typedef AbstractDriver<GC> Super;

  protected:
    typedef typename Super::OurGrid OurGrid;
    typedef typename Super::CC CC;

  private:

    Fonts m_fonts;

    bool paused;
    bool m_renderStats;
    u32 m_ticksLastStopped;

    s32 m_recordScreenshotPerAEPS;
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

    virtual void PostUpdate()
    {
      /* Update the stats renderer */
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());
      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
    }

    virtual void PostOnceOnly(VArguments& args)
    {
      if (m_countOfScreenshotsPerRate > 0) {
        m_maxRecordScreenshotPerAEPS = m_recordScreenshotPerAEPS;
        m_recordScreenshotPerAEPS = 1;
        m_countOfScreenshotsAtThisAEPS = 0;
      }

      SDL_Init(SDL_INIT_VIDEO);
      m_fonts.Init();

      m_rootPanel.SetName("Root");
      m_gridPanel.SetGridRenderer(&m_grend);
      m_gridPanel.SetGrid(&Super::GetGrid());

      m_statisticsPanel.SetStatsRenderer(&m_srend);
      m_statisticsPanel.SetGrid(&Super::GetGrid());
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());
      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
      m_statisticsPanel.SetVisibility(false);

      m_rootPanel.Insert(&m_gridPanel, NULL);
      m_gridPanel.Insert(&m_statisticsPanel, NULL);
      m_statisticsPanel.Insert(&m_buttonPanel, NULL);
      m_buttonPanel.SetVisibility(true);
      /*
      m_rootPanel.Insert(&m_panel1,0);
      m_rootPanel.Insert(&m_panel2,&m_panel1);
      m_panel1.Insert(&m_panel3,0);
      m_rootPanel.Insert(&m_panel4,0);
      m_panel5.SetControlled(&m_panel4);
      m_rootPanel.Insert(&m_panel5,0);
      */
      m_rootPanel.Print(STDOUT);

      m_srend.OnceOnly(m_fonts);

      SDL_WM_SetCaption("Movable Feast Machine Simulator", NULL);

      m_ticksLastStopped = 0;

    }


  private:

    void Update(OurGrid& grid)
    {
      KeyboardUpdate(grid);
      MouseUpdate(grid);
      Super::RunGrid(grid);
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
      m_grend.SetDimensions(Point<u32>(m_screenWidth - (m_renderStats ? STATS_WINDOW_WIDTH : 0)
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
	Super::DecrementAEPSPerFrame();
      }
      if(keyboard.IsDown(SDLK_PERIOD))
      {
	Super::IncrementAEPSPerFrame();
      }

      keyboard.Flip();
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
          for (OurGrid::iterator_type i = grid.begin(); i != grid.end(); ++i) {
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

    AbstractGUIDriver(int argc, const char** argv) :
      Super(argc, argv),
      m_renderStats(false),
      m_screenWidth(SCREEN_INITIAL_WIDTH),
      m_screenHeight(SCREEN_INITIAL_HEIGHT)
    {
      /* Needs to be called from here because of the virtual
	 override of PostOnceOnly. */
      Super::OnceOnly();
    }

    virtual void ReinitUs()
    {
      m_nextEventCountsAEPS = 0;
      m_nextScreenshotAEPS = 0;
      m_nextTimeBasedDataAEPS = 0;
    }

    virtual void PostReinit(VArguments& args)
    {
      HandleResize();

      m_renderStats = false;
    }

    virtual void HandleResize()
    { }

    void ToggleTileView()
    {
      m_grend.ToggleMemDraw();
    }

    /*********************************
     * GUI SPECIFIC DRIVER ARGUMENTS *
     *********************************/

    static void ConfigStatsOnlyView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;


      driver->m_screenWidth = STATS_START_WINDOW_WIDTH;
      driver->m_screenHeight = STATS_START_WINDOW_HEIGHT;
      driver->ToggleStatsView();
      driver->m_srend.SetDisplayAER(!driver->m_srend.GetDisplayAER());
    }

    static void ConfigMinimalView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_screenWidth = MINIMAL_START_WINDOW_HEIGHT;
      driver->m_screenHeight = MINIMAL_START_WINDOW_WIDTH;
    }

    static void SetRecordScreenshotPerAEPSFromArgs(const char* aeps, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_recordScreenshotPerAEPS = atoi(aeps);
    }

    static void SetPicturesPerRateFromArgs(const char* aeps, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_countOfScreenshotsPerRate = atoi(aeps);
    }

    virtual void AddDriverArguments(VArguments& args)
    {
      args.RegisterArgument("Start with the satistics view on the screen.",
			    "--startwithoutgrid", &ConfigStatsOnlyView, this, false);

      args.RegisterArgument("Start with the satistics view on the screen.",
			    "--startminimal", &ConfigMinimalView, this, false);

      args.RegisterArgument("Record screenshots every AEPS aeps",
			    "-p|--pictures", &SetRecordScreenshotPerAEPSFromArgs, this, true);

      args.RegisterArgument("Take COUNT shots per speed from 1 up to -p value",
			    "--picturesPerRate",
			    &SetPicturesPerRateFromArgs, this, true);

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
      OurGrid* m_grid;

    public:
      GridPanel() : m_grend(NULL), m_grid(NULL)
      {
	SetName("Grid Panel");
	SetDimensions(SCREEN_INITIAL_WIDTH,
		      SCREEN_INITIAL_HEIGHT);
	SetRenderPoint(SPoint(0, 0));
	SetForeground(Drawing::BLACK);
	SetBackground(Drawing::DARK_PURPLE);
      }

      void SetGrid(OurGrid* grid)
      {
	m_grid = grid;
      }

      void SetGridRenderer(GridRenderer* grend)
      {
	m_grend = grend;
      }

    protected:
      virtual void PaintComponent(Drawing& drawing)
      {
	this->Panel::PaintComponent(drawing);

	m_grend->RenderGrid(drawing, *m_grid);
      }

      virtual bool Handle(SDL_MouseButtonEvent& event)
      {
	if(event.button == SDL_BUTTON_LEFT)
	{
	  SPoint pt = GetAbsoluteLocation();
	  pt.Set(event.x - pt.GetX(),
		 event.y - pt.GetY());

	  m_grend->SelectTile(*m_grid,
			      pt);
	}
	return true;
      }
    } m_gridPanel;

    class StatisticsPanel : public Panel
    {
      StatsRenderer<GC>* m_srend;
      OurGrid* m_grid;
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
	SetRenderPoint(SPoint(100000, 0));
	SetForeground(Drawing::WHITE);
	SetBackground(Drawing::DARK_PURPLE);
	m_AEPS = m_AER = 0.0;
	m_aepsPerFrame = 0;
      }

      void SetStatsRenderer(StatsRenderer<GC>* srend)
      {
	m_srend = srend;
      }

      void SetGrid(OurGrid* grid)
      {
	m_grid = grid;
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
	m_srend->RenderGridStatistics(drawing, *m_grid,
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
	  LOG.Debug("(%d,%d)\n", pt.GetX(), pt.GetY());
	}
	return true;
      }
    }m_statisticsPanel;

    struct ButtonPanel : public Panel
    {
      ButtonPanel()
      {
	SetName("ButtonPanel");
	SetDimensions(STATS_START_WINDOW_WIDTH,
		      SCREEN_INITIAL_HEIGHT / 2);
	SetRenderPoint(SPoint(0,
			      SCREEN_INITIAL_HEIGHT / 2));
	SetForeground(Drawing::WHITE);
	SetBackground(Drawing::DARK_PURPLE);
      }

    }m_buttonPanel;

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

      UPoint newDimensions(width, height);

      m_rootPanel.SetDimensions(m_screenWidth, m_screenHeight);
      m_gridPanel.SetDimensions(m_screenWidth, m_screenHeight);

      m_rootPanel.SetRenderPoint(SPoint(0, 0));
      m_rootPanel.SetForeground(Drawing::BLUE);
      m_rootPanel.SetBackground(Drawing::RED);
      m_rootPanel.HandleResize(newDimensions);

      m_rootDrawing.Reset(screen, m_fonts.GetDefaultFont());

      if(m_renderStats)
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
      paused = Super::GetStartPaused();

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
                  m_rootPanel.Dispatch(event,
				       Rect(SPoint(),
					    UPoint(m_screenWidth,m_screenHeight)));
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


          Update(Super::GetGrid());

          m_rootDrawing.Clear();

          m_rootPanel.Paint(m_rootDrawing);

          if (m_recordScreenshotPerAEPS > 0) {
            if (!paused && Super::GetAEPS() >= m_nextScreenshotAEPS) {

              const char * path = Super::GetSimDirPathTemporary("vid/%010d.png",
								m_nextScreenshotAEPS);

              camera.DrawSurface(screen,path);
              {
                const char * path = Super::GetSimDirPathTemporary("tbd/data.dat");
                bool exists = true;
                {
                  FILE* fp = fopen(path, "r");
                  if (!fp) exists = false;
                  else fclose(fp);
                }
                FILE* fp = fopen(path, "a");
                FileByteSink fbs(fp);
                m_srend.WriteRegisteredCounts(fbs, !exists, Super::GetGrid(),
                                              Super::GetAEPS(),
					      Super::GetAER(),
					      Super::GetAEPSPerFrame(),
					      Super::GetOverheadPercent(), true);
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

	  if(Super::GetHaltAfterAEPS() > 0 &&
	     Super::GetAEPS() > Super::GetHaltAfterAEPS())
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

#endif /* ABSTRACTGUIDRIVER_H */
