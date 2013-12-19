#define MAIN_RUN_TESTS 1

#if MAIN_RUN_TESTS

#include "tests.h"

#else

#include <stdio.h>
#include "SDL/SDL.h"
#include "assert.h"
#include "eventwindow.h"
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

void update(Mouse* mouse, TileRenderer* tr)
{
  if(mouse->SemiAuto(SDL_BUTTON_LEFT))
  {
    tr->ToggleMemDraw();
  }
  
  mouse->Flip();
}

void RunSim()
{
  bool running = true;
  SDL_Surface* screen = 
    SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  SDL_Event event;
  TileRenderer renderer(screen);
  Point<int> renderPt(0, 0);
  Mouse mouse;

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


    update(&mouse, &renderer);

    Drawing::Clear(screen, 0xffffffff);
    
    Point<int> loc(0,0);
    Tile<P1Atom> tile(&loc);

    renderer.RenderTile<P1Atom>(&tile, &renderPt); 

    SDL_Flip(screen);
  }

  SDL_FreeSurface(screen);
}

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  //RunSim();

  ManhattanDir::AllocTables();

  Point<int> fromMan(2, 2);
  Point<int> toMan;

  u8 bits = ManhattanDir::FromPoint(&fromMan, false);

  printf("toBits: 0x%x\n", bits);

  ManhattanDir::FillFromBits(&toMan, bits, false);

  printf("FromBits: %d, %d", fromMan.GetX(),
	 fromMan.GetY());

  SDL_Quit();

  return 0;
}

#endif
