/* -*- C++ -*- */
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "itype.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "Point.h"

namespace MFM
{
  typedef enum
  {
    BUTTONFUNC_NOTHING = 0,
    BUTTONFUNC_TOGGLE_EXECUTION,
    BUTTONFUNC_EMPTY_TILE,
    BUTTONFUNC_RANDOM_NUKE,
    BUTTONFUNC_XRAY,
    BUTTONFUNC_TOGGLE_HEATMAP,
    BUTTONFUNC_TOGGLE_GRID,
    BUTTONFUNC_TOGGLE_TILEVIEW
  }ButtonFunction;

  class Button
  {
  private:

    static const u32 BUTTON_BORDER_SIZE = 2;
    static const u32 BUTTON_BORDER_COLOR = 0xffffffff;
    static const u32 BUTTON_COLOR = 0xff500050;
    
    SPoint m_dimensions;

    SPoint m_location;

    ButtonFunction m_func;

    const char* m_text;

  public:
    
    Button();

    Button(ButtonFunction func, const char* text, SPoint m_location, SPoint m_dimensions);

    ~Button();

    void Render(SDL_Surface* sfc, SPoint& offset, TTF_Font* font);

    ButtonFunction Contains(SPoint& pt);
  };
}

#endif /* ABSTRACTBUTTON_H */
