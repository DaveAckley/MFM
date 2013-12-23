#define MAIN_RUN_TESTS 1

#if MAIN_RUN_TESTS

#include <boost/test/minimal.hpp>
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

class MFMSim
{
private:

  Mouse mouse;
  Keyboard keyboard;
  SDL_Surface* screen;
  GridRenderer grend;

  void Update()
  { 
    if(keyboard.SemiAuto(SDLK_1))
    {
      grend.IncreaseAtomSize();
    }
    if(keyboard.SemiAuto(SDLK_2))
    {
      grend.DecreaseAtomSize();
    }

    mouse.Flip();
    keyboard.Flip();
  }

public:

  MFMSim() { }

  void Run()
  {
    bool running = true;
    screen = SDL_SetVideoMode(256, 256, 32,
			      SDL_SWSURFACE);

    SDL_Event event;
    grend.SetDestination(screen);
    Grid<P1Atom> mainGrid(4, 4);

    mainGrid.SetStateFunc(&P1Atom::StateFunc);

    P1Atom atom(ELEMENT_DREG);
    Point<int> aloc(10, 10);

    mainGrid.PlaceAtom(&atom, &aloc);

    P1Atom* other = mainGrid.GetAtom(&aloc);

    printf("%d == %d?\n", atom.GetState(), other->GetState());
    assert(other->GetState() == atom.GetState());


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
      
      
      Update();
      
      Drawing::Clear(screen, 0xffffffff);
      
      grend.RenderGrid(&mainGrid);
      
      SDL_Flip(screen);
    }
    
    SDL_FreeSurface(screen);
  }
};


int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  ManhattanDir::AllocTables();

  MFMSim sim;

  sim.Run();

  SDL_Quit();

  return 0;
}

#endif
