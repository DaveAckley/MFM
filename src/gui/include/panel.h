#ifndef PANEL_H     /* -*- C++ -*- */
#define PANEL_H

#include "itype.h"
#include "point.h"
#include "SDL/SDL.h"

class Panel
{
protected:

  u32 m_width, m_height;

  Point<s32> m_renderPt;

  SDL_Surface* m_dest;

public:

  Panel();

  Panel(u32 width, u32 height);

  Panel(SDL_Surface* dest);

  Panel(SDL_Surface* dest, u32 width, u32 height);

  void SetDestination(SDL_Surface* dest);

  u32 GetWidth() {return m_width;}

  u32 GetHeight() {return m_height;}

  SDL_Surface* GetDestination() {return m_dest;}

  void SetDimensions(u32 width, u32 height);

  void SetRenderPoint(Point<s32>& renderPt);

  void Render(void* rendObj);

};

#endif /*PANEL_H*/
