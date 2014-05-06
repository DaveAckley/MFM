/* -*- C++ -*- */
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "itype.h"
//#include "SDL/SDL.h"
//#include "SDL/SDL_ttf.h"
#include "Point.h"
#include "Panel.h"
#include "Drawing.h"

namespace MFM
{
  class AbstractButton : public Panel
  {
  private:

    static const u32 BUTTON_BORDER_SIZE = 2;
    static const u32 BUTTON_BORDER_COLOR = 0xffffffff;
    static const u32 BUTTON_COLOR = 0xff600060;

    SPoint m_dimensions;

    SPoint m_location;

    const char* m_text;

    bool m_enabled;

  public:

    AbstractButton();

    AbstractButton(const char* text);

    void SetLocation(const SPoint& location)
    {
      m_location = location;
    }

    void SetDimensions(const SPoint& dimensions)
    {
      m_dimensions = dimensions;
    }

    void SetEnabled(bool isEnabled)
    {
      m_enabled = isEnabled;
    }

    bool IsEnabled() const
    {
      return m_enabled;
    }

    /////////
    //// Panel Methods

    virtual bool Handle(SDL_MouseButtonEvent & event)
    {
      if (IsEnabled() && event.type == SDL_MOUSEBUTTONUP) {
        OnClick();
        return true;
      }
      return false;
    }

    virtual void PaintComponent(Drawing & config) ;

    virtual void OnClick() = 0;

    ~AbstractButton();

    void Render(Drawing & drawing, SPoint& offset, TTF_Font* font);

    bool Contains(SPoint& pt);
  };
}

#endif /* ABSTRACTBUTTON_H */
