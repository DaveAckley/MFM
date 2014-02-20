#include "main.h"

namespace MFM {

#define FRAMES_PER_SECOND 100.0

#define EVENTS_PER_FRAME 100000

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024

#define STATS_WINDOW_WIDTH 256

class MFMSim
{
public:
  static const u32 EVENT_WINDOW_RADIUS = 4;
  static const u32 GRID_WIDTH = 5;
  static const u32 GRID_HEIGHT = 3;
  static const u32 ELEMENT_TABLE_BITS = 8;


private:

  typedef Grid<P1Atom,EVENT_WINDOW_RADIUS,GRID_WIDTH,GRID_HEIGHT> GridP1Atom;
  typedef ElementTable<P1Atom,EVENT_WINDOW_RADIUS,ELEMENT_TABLE_BITS> ElementTableP1Atom;

  bool paused;

  bool renderStats;

  u32 m_eventsPerFrame;
  double m_AEPS;

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
      u32 processingTime = SDL_GetTicks();
      for(u32 i = 0; i < m_eventsPerFrame; i++)
      {
	grid.TriggerEvent();
      }
      processingTime = SDL_GetTicks() - processingTime;

      /* m_eventProcessingTime is measured in milliseconds, so convert to seconds.*/
      m_AEPS = (double)m_eventsPerFrame / (processingTime / 1000.0);
    }

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim() 
  {
    m_eventsPerFrame = EVENTS_PER_FRAME;
    m_AEPS = 0;
  }

  void Run(u32 seedOrZero)
  {
    paused = true;

    bool running = true;
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			      SDL_SWSURFACE);

    SDL_Event event;
    
    GridP1Atom mainGrid;

    mainGrid.Needed(Element_Empty<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Dreg<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Res<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Sorter<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Emitter<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Consumer<P1Atom, 4>::THE_INSTANCE);
    mainGrid.Needed(Element_Data<P1Atom, 4>::THE_INSTANCE);

    if (seedOrZero==0) seedOrZero = 1;  /* Avoid superstitious 0 zeed */
    mainGrid.SetSeed(seedOrZero);  /* Push seeds out to everybody */

    grend.SetDestination(screen);
    grend.SetDimensions(UPoint(SCREEN_WIDTH,SCREEN_HEIGHT));

    srend.SetDestination(screen);
    srend.SetDrawPoint(SPoint(1024, 0));
    srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, SCREEN_HEIGHT));

    renderStats = false;

    P1Atom atom(Element_Dreg<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
    P1Atom sorter(Element_Sorter<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
    P1Atom emtr(Element_Emitter<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
    P1Atom cnsr(Element_Consumer<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

    emtr.SetStateField(0,10,10);  // What is this for??
    cnsr.SetStateField(0,10,10);  // What is this for??

    sorter.SetStateField(0,32,50);  // Default threshold

    u32 realWidth = TILE_WIDTH - EVENT_WINDOW_RADIUS * 2;

    SPoint aloc(20, 30);
    SPoint sloc(20, 10);
    SPoint eloc(GRID_WIDTH*realWidth-2, 10);
    SPoint cloc(1, 10);

    for(u32 x = 0; x < mainGrid.GetWidth(); x++)
    {
      for(u32 y = 0; y < mainGrid.GetHeight(); y++)
      {
	for(u32 z = 0; z < 4; z++)
	{
	  aloc.Set(20 + x * realWidth + z, 20 + y * realWidth);
	  sloc.Set(21 + x * realWidth + z, 21 + y * realWidth);
	  mainGrid.PlaceAtom(sorter, sloc);
	  mainGrid.PlaceAtom(atom, aloc);
	}
      }
    }
    
    mainGrid.PlaceAtom(emtr, eloc);
    mainGrid.PlaceAtom(cnsr, cloc);
    mainGrid.PlaceAtom(cnsr, cloc+SPoint(1,1));  // More consumers than emitters!

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
      
      
      Update(mainGrid);
      
      Drawing::Clear(screen, 0xff200020);
      
      grend.RenderGrid(mainGrid);
      if(renderStats)
      {
	srend.RenderGridStatistics(mainGrid, m_AEPS);
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

