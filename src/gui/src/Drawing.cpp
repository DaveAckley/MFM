#include <cmath>
#include "Drawing.h"

namespace MFM {

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
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = y;
  SDL_FillRect(dest, &rect, color);
}

void Drawing::FillCircle(SDL_Surface* dest, int x, int y,
			 int w, int h, int radius,
			 Uint32 color)
{
  SPoint ip;
  SPoint cp(x + (w / 2), y + (h / 2));
  for(int i = x; i < x + w; i++)
  {
    for(int j = y; j < y + h; j++)
    {
      ip.Set(i, j);
      ip.Subtract(cp);
      if(ip.GetEuclideanLength() < radius)
      {
	SetPixel(dest, i, j, color);
      }
    }
  }
}

void Drawing::BlitText(SDL_Surface*& dest, TTF_Font*& font,
		       const char* message, Point<u32> loc,
		       Point<u32> size, u32 color)
{
  SDL_Color sdl_color;
  sdl_color.r = (color >> 16) & 0xff;
  sdl_color.g = (color >> 8) & 0xff;
  sdl_color.b = color & 0xff;

  SDL_Surface* text = TTF_RenderText_Blended(font, message, sdl_color);

  SDL_Rect rect;
  rect.x = loc.GetX();
  rect.y = loc.GetY();
  rect.w = size.GetX();
  rect.h = size.GetY();

  SDL_BlitSurface(text, NULL, dest, &rect);

  SDL_FreeSurface(text);
}
} /* namespace MFM */

