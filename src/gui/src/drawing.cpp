#include <cmath>
#include "drawing.h"

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

void Drawing::FillCircle(SDL_Surface* dest, int x, int y,
			 int w, int h, int radius,
			 Uint32 color)
{
  Point<int> ip;
  Point<int> cp(x + (w / 2), y + (h / 2));
  for(int i = x; i < x + w; i++)
  {
    for(int j = y; j < y + h; j++)
    {
      ip.Set(i, j);
      ip.Subtract(cp);
      if(ip.GetLength() < radius)
      {
	SetPixel(dest, i, j, color);
      }
    }
  }
}
