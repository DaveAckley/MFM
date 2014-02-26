#ifndef DRAWING_H      /* -*- C++ -*- */
#define DRAWING_H

#include "itype.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "Tile.h"

namespace MFM {

class Drawing
{
public:

  static const int WHITE   = 0xffffffff;
  static const int RED     = 0xffff0000;
  static const int GREEN   = 0xff00ff00;
  static const int BLUE    = 0xff0000ff;
  static const int CYAN    = 0xff00ffff;
  static const int MAGENTA = 0xffff00ff;
  static const int YELLOW  = 0xffffff00;

  static inline void SetPixel(SDL_Surface* dest,
			      u32 x, u32 y,
			      u32 color)
  {
    if(x >= 0 && y >= 0 && x < (u32)dest->w && y < (u32)dest->h)
    {
      ((u32*)dest->pixels)[x + y * dest->w] = color;
    }
  }

  static inline void SetPixel(SDL_Surface* dest,
			      int i, u32 color);

  static void Clear(SDL_Surface* dest, u32 color);

  static void DrawHLine(SDL_Surface* dest,
			int y, int startX, int endX,
			u32 color);

  static void DrawVLine(SDL_Surface* dest,
			int x, int startY, int endY,
			u32 color);

  static void FillRect(SDL_Surface* dest,
		       int x, int y, int w, int h,
		       u32 color);

  static void FillCircle(SDL_Surface* dest,
			 int x, int y, int w,
			 int h, int radius,
			 u32 color);

  static void BlitText(SDL_Surface*& dest, TTF_Font*& font,
		       const char* message, Point<u32> loc,
		       Point<u32> size, u32 color);

};
} /* namespace MFM */

#endif /*DRAWING_H*/

