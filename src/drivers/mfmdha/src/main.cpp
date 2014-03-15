#include "main.h"

namespace MFM {

#define FRAMES_PER_SECOND 100.0

#define EVENTS_PER_FRAME 100000

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define SCREEN_INITIAL_WIDTH 1280
#define SCREEN_INITIAL_HEIGHT 1024

#define SCREEN_BPP 32

#define STATS_WINDOW_WIDTH 256

class MFMSim
{
public:
  static const u32 EVENT_WINDOW_RADIUS = 4;
  static const u32 GRID_WIDTH = 7;
  static const u32 GRID_HEIGHT = 4;
  static const u32 ELEMENT_TABLE_BITS = 3;


private:

  typedef Grid<P1Atom,EVENT_WINDOW_RADIUS,GRID_WIDTH,GRID_HEIGHT> GridP1Atom;
  typedef ElementTable<P1Atom,EVENT_WINDOW_RADIUS,ELEMENT_TABLE_BITS> ElementTableP1Atom;

  bool paused;

  bool renderStats;

  u32 m_eventsPerFrame;
  double m_AEPS;
  double m_AER;
  u64 m_msSpentRunning;

  Mouse mouse;
  Keyboard keyboard;
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

  void Update(GridP1Atom& grid)
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
    if(keyboard.IsDown(SDLK_RIGHT))
    {
      m_grend.MoveRight(speed);
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
      Sleep(0, 100000000);
      grid.Pause();
      m_msSpentRunning += (SDL_GetTicks() - startMS);
 
      m_AEPS = (double) grid.GetTotalEventsExecuted() / grid.GetTotalSites();
      m_AER = 1000 * m_AEPS / m_msSpentRunning;
    }

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim()
  {
    m_eventsPerFrame = EVENTS_PER_FRAME;
    m_AEPS = 0;
    m_AER = 0;
    m_msSpentRunning = 0;
  }

  void SetScreenSize(u32 width, u32 height) {
    m_screenWidth = width;
    m_screenHeight = height;
    screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, SCREEN_BPP,
			      SDL_SWSURFACE | SDL_RESIZABLE);
    if (screen == 0) 
      FAIL(ILLEGAL_STATE);

    m_grend.SetDestination(screen);
    m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));

    m_srend.SetDestination(screen);
    m_srend.SetDrawPoint(SPoint(m_screenWidth-STATS_WINDOW_WIDTH, 0));
    m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));
  }

  void Run(u32 seedOrZero)
  {
    paused = true;

    bool running = true;
    SetScreenSize(SCREEN_INITIAL_WIDTH, SCREEN_INITIAL_HEIGHT);

    SDL_Event event;

    GridP1Atom mainGrid;
    for (u32 y = 0; y < mainGrid.GetHeight(); ++y) {
      for (u32 x = 0; x < mainGrid.GetWidth(); ++x) {
        fprintf(stderr,"  (%2d,%2d):%p",x,y,(void*) &mainGrid.GetTile(SPoint(x,y)));
      }
      fprintf(stderr,"\n");
    }

    mainGrid.Needed(Element_Empty<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Dreg<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Res<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Boids1<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Boids2<P1Atom, 4>::THE_INSTANCE);

    if (seedOrZero==0) seedOrZero = 1;  /* Avoid superstitious 0 zeed */
    mainGrid.SetSeed(seedOrZero);  /* Push seeds out to everybody */

    m_srend.DisplayStatsForType(Element_Empty<P1Atom, 4>::TYPE);
    m_srend.DisplayStatsForType(Element_Dreg<P1Atom, 4>::TYPE);
    m_srend.DisplayStatsForType(Element_Res<P1Atom, 4>::TYPE);
    m_srend.DisplayStatsForType(Element_Boids1<P1Atom, 4>::TYPE);
    m_srend.DisplayStatsForType(Element_Boids2<P1Atom, 4>::TYPE);

    renderStats = false;

    P1Atom aBoid1(Element_Boids1<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
    P1Atom aBoid2(Element_Boids2<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
    P1Atom aDReg(Element_Dreg<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

    u32 realWidth = Tile<P1Atom,4>::OWNED_SIDE;

    SPoint aloc(20, 30);
    SPoint sloc(20, 10);
    SPoint eloc(GRID_WIDTH*realWidth-2, 10);
    SPoint cloc(1, 10);

    for(u32 x = 0; x < mainGrid.GetWidth()*realWidth; x+=4) {
      for(u32 y = 0; y < mainGrid.GetHeight()*realWidth; y+=4) {
        aloc.Set(x,y);
        sloc.Set(x+1,y+1);

        if ((x/4)&1)
          mainGrid.PlaceAtom(aBoid1, aloc);
        else
          mainGrid.PlaceAtom(aBoid2, aloc);
        mainGrid.PlaceAtom(aDReg, sloc);
      }
    }

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

      SDL_Flip(screen);
    }

    SDL_FreeSurface(screen);
  }
};
} /* namespace MFM */

int main(int argc, char** argv)
{
  MFM::u32 seed;
  switch (argc) {
  case 1:
    seed = time(NULL);
    break;
  case 2:
    seed = atoi(argv[1]);
    break;
  default:
    fprintf(stderr,"Too many arguments (%d), 0 or 1 needed\n",argc);
    exit(1);
  }

  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  SDL_WM_SetCaption("Movable Feast Machine Simulator", NULL);

  printf("[Seed=%d]\n", seed);

  MFM::MFMSim sim;

  sim.Run(seed);

  SDL_Quit();

  return 0;
}

