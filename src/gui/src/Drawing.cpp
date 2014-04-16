#include <cmath>
#include "Drawing.h"

namespace MFM {

  void Drawing::Clear(SDL_Surface* dest, Uint32 color)
  {
    SDL_FillRect(dest, 0, color);
  }

  void Drawing::DrawHLine(SDL_Surface* dest, int y,
                          int startX, int endX,
                          Uint32 color)
  {
    FillRect(dest,startX,y,endX-startX,1,color);
  }

  void Drawing::DrawVLine(SDL_Surface* dest, int x,
                          int startY, int endY,
                          Uint32 color)
  {
    FillRect(dest,x,startY,1,endY-startY,color);
  }

  void Drawing::FillRect(SDL_Surface* dest, int x, int y,
                         int w, int h, Uint32 color)
  {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_FillRect(dest, &rect, color);
  }

  void Drawing::FillCircle(SDL_Surface* dest, int x, int y,
                           int w, int h, int radius,
                           Uint32 color)
  {
    double cx = x+w/2.0;
    double cy = y+h/2.0;
    for(int dy = 1; dy <= radius; dy++)
      {
        double dx = floor(sqrt(2.0*radius*dy - dy*dy));
        DrawHLine(dest, (int) (cy + radius - dy), (int) (cx-dx), (int)(cx+dx), color);
        DrawHLine(dest, (int) (cy - radius + dy), (int) (cx-dx), (int)(cx+dx), color);
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

