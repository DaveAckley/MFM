#ifndef DRAWING_H
#define DRAWING_H

#include "SDL/SDL.h"
#include "tile.h"

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
			      int x, int y,
			      Uint32 color);

  static inline void SetPixel(SDL_Surface* dest,
			      int i, Uint32 color);

  static void Clear(SDL_Surface* dest, Uint32 color);

  static void DrawHLine(SDL_Surface* dest,
			int y, int startX, int endX,
			Uint32 color);

  static void DrawVLine(SDL_Surface* dest,
			int x, int startY, int endY,
			Uint32 color);

  static void FillRect(SDL_Surface* dest,
		       int x, int y, int w, int h,
		       Uint32 color);
};

#endif /*DRAWING_H*/
