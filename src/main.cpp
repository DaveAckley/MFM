#include <stdio.h>
#include "SDL/SDL.h"
#include "assert.h"
#include "eventwindow.h"
#include "point.h"
#include "drawing.h"
#include "tilerenderer.h"

void runSim()
{
  bool running = true;
  SDL_Surface* screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
  SDL_Event event;
  TileRenderer renderer(screen);
  Point<int> renderPt(0, 0);

  Drawing::Clear(screen, 0xffff0000);

  while(running)
  {
    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
      case SDL_QUIT:
	running = false;
	break;
      }
    }
    
    renderer.RenderTile(NULL, &renderPt); 

    SDL_Flip(screen);
  }

  SDL_FreeSurface(screen);
}

int main(int argc, char** argv)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  runSim();

  SDL_Quit();

  return 0;
}
