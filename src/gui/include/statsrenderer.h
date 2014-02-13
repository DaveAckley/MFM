#ifndef STATSRENDERER_H /* -*- C++ -*- */
#define STATSRENDERER_H

#include "SDL/SDL_ttf.h"
#include "grid.h"
#include "itype.h"
#include "point.h"

namespace MFM {

class StatsRenderer
{
private:
  Point<u32> m_dimensions;
  Point<s32> m_drawPoint;

  TTF_Font* m_drawFont;

  SDL_Surface* m_dest;

public:

  StatsRenderer()
  {
    m_drawFont = TTF_OpenFont("tiny.ttf", 20);
  }

  ~StatsRenderer()
  {
    TTF_CloseFont(m_drawFont);
  }

  void SetDestination(SDL_Surface* dest)
  {
    m_dest = dest;
  }

  void SetDrawPoint(Point<s32> drawPoint)
  {
    m_drawPoint = drawPoint;
  }

  void SetDimensions(Point<u32> dimensions)
  {
    m_dimensions = dimensions;
  }
  
  template <class T, u32 R>
  void DrawString(const char* string, u32 x, u32 y);

  template<class T, u32 R>
  void RenderGridStatistics(Grid<T,R>& grid, double aeps);
};
} /* namespace MFM */
#include "statsrenderer.tcc"

#endif /*STATSRENDERER_H*/

