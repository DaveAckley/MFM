#define MAIN_RUN_TESTS 0

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
#include "manhattandir.h"
#include "mouse.h"
#include "p1atom.h"
#include "point.h"
#include "drawing.h"
#include "tilerenderer.h"

#define FRAMES_PER_SECOND 60.0

void HandleMouse(Mouse* mouse, SDL_MouseButtonEvent* e)
{
  if(e->state == SDL_PRESSED)
  {
    mouse->Press(e->button);
  }
  else if(e->state == SDL_RELEASED)
  {
    mouse->Release(e->button);
  }
}

void update(Mouse* mouse)
{ 
  mouse->Flip();
}

void RunSim()
{
  bool running = true;
  SDL_Surface* screen = 
    SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  SDL_Event event;
  GridRenderer grend(screen);
  Mouse mouse;

  Grid<P1Atom> mainGrid(2, 2);

  int lastFrame = SDL_GetTicks();

  Drawing::Clear(screen, 0xffffffff);

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
	HandleMouse(&mouse, &event.button);
	break;
      }
    }

    /* Limit framerate */
    int sleepMS = (int)((1000.0 / FRAMES_PER_SECOND) - 
			(SDL_GetTicks() - lastFrame));
    if(sleepMS > 0)
    {
      SDL_Delay(sleepMS);
    }
    lastFrame = SDL_GetTicks();


    update(&mouse);

    Drawing::Clear(screen, 0xffffffff);

    grend.RenderGrid(&mainGrid);

    SDL_Flip(screen);
  }

  SDL_FreeSurface(screen);
}

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  RunSim();

  SDL_Quit();

  return 0;
}

#endif
