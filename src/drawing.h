#ifndef DRAWING_H
#define DRAWING_H

#include "SDL/SDL.h"
#include "tile.h"

class Drawing
{
 public:
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
};

#endif /*DRAWING_H*/
