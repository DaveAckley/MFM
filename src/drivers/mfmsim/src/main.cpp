#include "main.h"

#define FRAMES_PER_SECOND 60.0

#define EVENTS_PER_FRAME 100000

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define STATS_WINDOW_WIDTH 256

class MFMSim
{
public:
  static const u32 EVENT_WINDOW_RADIUS = 4;

private:

  typedef Grid<P1Atom,EVENT_WINDOW_RADIUS> GridP1Atom;
  typedef ElementTable<P1Atom,EVENT_WINDOW_RADIUS> ElementTableP1Atom;

  bool paused;

  bool renderStats;

  u32 m_eventsPerFrame;

  Mouse mouse;
  Keyboard keyboard;
  SDL_Surface* screen;
  GridRenderer grend;
  StatsRenderer srend;

  void Update(GridP1Atom& grid)
  { 
    u8 speed = keyboard.ShiftHeld() ?
      CAMERA_FAST_SPEED : CAMERA_SLOW_SPEED;

    if(keyboard.IsDown(SDLK_1))
    {
      grend.IncreaseAtomSize();
    }
    if(keyboard.IsDown(SDLK_2))
    {
      grend.DecreaseAtomSize();
    }
    if(keyboard.SemiAuto(SDLK_g))
    {
      grend.ToggleGrid();
    }
    if(keyboard.SemiAuto(SDLK_m))
    {
      grend.ToggleMemDraw();
    }
    if(keyboard.SemiAuto(SDLK_l))
    {
      grend.ToggleDataHeatmap();
    }
    if(keyboard.SemiAuto(SDLK_p))
    {
      grend.ToggleTileSeparation();
    }
    if(keyboard.SemiAuto(SDLK_o))
    {
      grend.SetEventWindowRenderMode(EVENTWINDOW_RENDER_OFF);
    }
    if(keyboard.SemiAuto(SDLK_e))
    {
      grid.TriggerEvent();
    }
    if(keyboard.IsDown(SDLK_LEFT) ||
       keyboard.IsDown(SDLK_a))
    {
      grend.MoveLeft(speed);
    }
    if(keyboard.IsDown(SDLK_DOWN) ||
       keyboard.IsDown(SDLK_s))
    {
      grend.MoveDown(speed);
    }
    if(keyboard.IsDown(SDLK_UP) ||
       keyboard.IsDown(SDLK_w))
    {
      grend.MoveUp(speed);
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
	grend.SetDimensions(Point<u32>(SCREEN_WIDTH, SCREEN_HEIGHT));
      }
      else
      {
	grend.SetDimensions(Point<u32>(SCREEN_WIDTH - STATS_WINDOW_WIDTH,
				       SCREEN_HEIGHT));
      }
    }
    if(keyboard.IsDown(SDLK_RIGHT) ||
       keyboard.IsDown(SDLK_d))
    {
      grend.MoveRight(speed);
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
      for(u32 i = 0; i < m_eventsPerFrame; i++)
      {
	grid.TriggerEvent();
      }
    }

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim() 
  {
    m_eventsPerFrame = EVENTS_PER_FRAME;
  }

  void Run()
  {
    // Repeatable until forced otherwise
    srandom(3);

    paused = true;

    bool running = true;
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			      SDL_SWSURFACE);

    ElementTableP1Atom elements(&P1Atom::StateFunc);

    SDL_Event event;
    
    GridP1Atom mainGrid(4, 2, &elements);

    grend.SetDestination(screen);
    grend.SetDimensions(Point<u32>(SCREEN_WIDTH,SCREEN_HEIGHT));

    srend.SetDestination(screen);
    srend.SetDrawPoint(Point<s32>(1024, 0));
    srend.SetDimensions(Point<u32>(STATS_WINDOW_WIDTH, SCREEN_HEIGHT));

    mainGrid.SetStateFunc(&P1Atom::StateFunc);

    renderStats = false;

    P1Atom atom(ELEMENT_DREG);
    P1Atom sorter(ELEMENT_SORTER);
    P1Atom emtr(ELEMENT_EMITTER);
    P1Atom cnsr(ELEMENT_CONSUMER);

    emtr.WriteLowerBits(3);

    sorter.WriteLowerBits(50);

    Point<int> aloc(70, 30);
    Point<int> sloc(10, 10);
    Point<int> eloc(131, 10);
    Point<int> cloc(4, 10);

    for(int i = 0; i < 10; i++)
    {
      mainGrid.PlaceAtom(atom, aloc);
      aloc.SetX(aloc.GetX() + 1);
    }
    
    mainGrid.PlaceAtom(sorter, sloc);
    mainGrid.PlaceAtom(emtr, eloc);
    mainGrid.PlaceAtom(cnsr, cloc);

    s32 lastFrame = SDL_GetTicks();

    while(running)
    {
      while(SDL_PollEvent(&event))
      {
	switch(event.type)
	{
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
      
      
      if(rand() % 3 == 0)
      Update(mainGrid);
      
      Drawing::Clear(screen, 0xff200020);
      
      grend.RenderGrid(mainGrid);
      if(renderStats)
      {
	srend.RenderGridStatistics(mainGrid);
      }
      
      SDL_Flip(screen);
    }
    
    SDL_FreeSurface(screen);
  }
};

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  SDL_WM_SetCaption("Movable Feast Machine Simulator", NULL);

  srand(time(NULL));

  //ManhattanDir<4>::AllocTables(EVENT_WINDOW_RADIUS);

  MFMSim sim;

  sim.Run();

  //ManhattanDir<4>::DeallocTables();

  SDL_Quit();

  return 0;
}
