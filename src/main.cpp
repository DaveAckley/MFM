#define MAIN_RUN_TESTS 0

#if MAIN_RUN_TESTS

#include "tests.h"

#else

#include <stdio.h>
#include "SDL/SDL.h"
#include "assert.h"
#include "eventwindow.h"
#include "grid.h"
#include "gridrenderer.h"
#include "itype.h"
#include "keyboard.h"
#include "manhattandir.h"
#include "mouse.h"
#include "p1atom.h"
#include "point.h"
#include "drawing.h"
#include "tilerenderer.h"
#include "time.h"

#define FRAMES_PER_SECOND 60.0

#define EVENTS_PER_FRAME 0

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 10

class MFMSim
{
public:
  static const u32 EVENT_WINDOW_RADIUS = 2;

private:

  typedef Grid<P1Atom,EVENT_WINDOW_RADIUS> GridP1Atom;
  typedef ElementTable<P1Atom,EVENT_WINDOW_RADIUS> ElementTableP1Atom;

  Mouse mouse;
  Keyboard keyboard;
  SDL_Surface* screen;
  GridRenderer grend;

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
    if(keyboard.SemiAuto(SDLK_p))
    {
      grend.ToggleTileSeparation();
    }
    if(keyboard.SemiAuto(SDLK_o))
    {
      grend.SetEventWindowRenderMode(EVENTWINDOW_RENDER_OFF);
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
      grid.TriggerEvent();
    }
    if(keyboard.IsDown(SDLK_RIGHT) ||
       keyboard.IsDown(SDLK_d))
    {
      grend.MoveRight(speed);
    }

    for(u32 i = 0; i < EVENTS_PER_FRAME; i++)
    {
      grid.TriggerEvent();
    }

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim() { }

  void Run()
  {
    // Repeatable until forced otherwise
    srandom(5);

    bool running = true;
    screen = SDL_SetVideoMode(640, 640, 32,
			      SDL_SWSURFACE);

    ElementTableP1Atom elements(&P1Atom::StateFunc);

    SDL_Event event;
    grend.SetDestination(screen);
    GridP1Atom mainGrid(3, 3, &elements);

    mainGrid.SetStateFunc(&P1Atom::StateFunc);

    P1Atom atom(ELEMENT_DREG);
    P1Atom res(ELEMENT_RES);
    Point<int> aloc(3, 3);
    Point<int> rloc(16, 16);

    /* 
     * Getto fix for keeping atoms from going
     * into the cache
     */

    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
	if(x < EVENT_WINDOW_RADIUS ||
	   y < EVENT_WINDOW_RADIUS ||
	   x >= TILE_WIDTH - EVENT_WINDOW_RADIUS ||
	   y >= TILE_WIDTH - EVENT_WINDOW_RADIUS)
	{
	  Point<int> nloc(x, y);

	  mainGrid.PlaceAtom(res, nloc);
	}
      }
    }

    mainGrid.PlaceAtom(atom, aloc);
    mainGrid.PlaceAtom(res, rloc);

    int lastFrame = SDL_GetTicks();

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
      int sleepMS = (int)
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
      
      SDL_Flip(screen);
    }
    
    SDL_FreeSurface(screen);
  }
};


int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  srand(time(NULL));

  //ManhattanDir<4>::AllocTables(EVENT_WINDOW_RADIUS);

  MFMSim sim;

  sim.Run();

  //ManhattanDir<4>::DeallocTables();

  SDL_Quit();

  return 0;
}

#endif
