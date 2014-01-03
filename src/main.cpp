#define MAIN_RUN_TESTS 1

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
#include "keyboard.hpp"
#include "manhattandir.h"
#include "mouse.h"
#include "p1atom.h"
#include "point.h"
#include "drawing.h"
#include "tilerenderer.h"

#define FRAMES_PER_SECOND 60.0

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 10

class MFMSim
{
private:

  Mouse mouse;
  Keyboard keyboard;
  SDL_Surface* screen;
  GridRenderer grend;

  void Update(Grid<P1Atom>& grid)
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

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim() { }

  void Run()
  {
    bool running = true;
    screen = SDL_SetVideoMode(640, 640, 32,
			      SDL_SWSURFACE);

    ElementTable<P1Atom> elements(&P1Atom::StateFunc);

    SDL_Event event;
    grend.SetDestination(screen);
    Grid<P1Atom> mainGrid(1, 1, &elements);

    mainGrid.SetStateFunc(&P1Atom::StateFunc);

    P1Atom atom(ELEMENT_DREG);
    Point<int> aloc(10, 10);

    mainGrid.PlaceAtom(atom, aloc);

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
      
      Drawing::Clear(screen, 0xffffffff);
      
      grend.RenderGrid(mainGrid);
      
      SDL_Flip(screen);
    }
    
    SDL_FreeSurface(screen);
  }
};


int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  ManhattanDir::AllocTables(EVENT_WINDOW_RADIUS);

  MFMSim sim;

  sim.Run();

  ManhattanDir::DeallocTables();

  SDL_Quit();

  return 0;
}

#endif
