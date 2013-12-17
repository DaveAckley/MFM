#include <stdio.h>
#include "SDL/SDL.h"
#include "assert.h"
#include "eventwindow.h"
#include "mouse.h"
#include "point.h"
#include "drawing.h"
#include "tilerenderer.h"

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
  SDL_Surface* screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
  SDL_Event event;
  TileRenderer renderer(screen);
  Point<int> renderPt(0, 0);
  Mouse mouse;

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

    update(&mouse, &renderer);

    Drawing::Clear(screen, 0xffffffff);
    
    renderer.RenderTile(NULL, &renderPt); 

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
