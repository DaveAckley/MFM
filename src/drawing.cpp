#include "drawing.h"

inline void Drawing::SetPixel(SDL_Surface* dest, int x,
		       int y, Uint32 color)
{
  if(x >= 0 && y >= 0 && x < dest->w && y < dest->h)
  {
    ((Uint32*)dest->pixels)[x + y * dest->w] = color;
  }
}

void Drawing::SetPixel(SDL_Surface* dest, int i,
		       Uint32 color)
{
  if(i >= 0 && i < dest->w * dest->h)
  {
    ((Uint32*)dest->pixels)[i] = color;
  }
}

void Drawing::Clear(SDL_Surface* dest, Uint32 color)
{
  for(int i = 0; i < dest->w * dest->h; i++)
  {
    SetPixel(dest, i, color);
  }
}

void Drawing::DrawHLine(SDL_Surface* dest, int y,
			int startX, int endX,
			Uint32 color)
{
  for(int x = startX; x < endX; x++)
  {
    SetPixel(dest, x, y, color);
  }
}

void Drawing::DrawVLine(SDL_Surface* dest, int x,
			int startY, int endY,
			Uint32 color)
{
  for(int y = startY; y < endY; y++)
  {
    SetPixel(dest, x, y, color);
  }
}

void Drawing::FillRect(SDL_Surface* dest, int x, int y,
		       int w, int h, Uint32 color)
{
  for(int i = x; i < x + w; i++)
  {
    for(int j = y; j < y + h; j++)
    {
      SetPixel(dest, i, j, color);
    }
  }
}
