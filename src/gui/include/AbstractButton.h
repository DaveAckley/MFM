/* -*- C++ -*- */
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "itype.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "Point.h"

namespace MFM
{
  class AbstractButton
  {
  private:

    static const u32 BUTTON_BORDER_SIZE = 2;
    static const u32 BUTTON_BORDER_COLOR = 0xffffffff;
    static const u32 BUTTON_COLOR = 0xff600060;
    
    SPoint m_dimensions;

    SPoint m_location;

    const char* m_text;

  public:
    
    AbstractButton();

    AbstractButton(const char* text);

    void SetLocation(SPoint& location)
    {
      m_location = location;
    }

    void SetDimensions(SPoint& dimensions)
    {
      m_dimensions = dimensions;
    }

    virtual void OnClick() = 0;

    ~AbstractButton();

    void Render(SDL_Surface* sfc, SPoint& offset, TTF_Font* font);

    bool Contains(SPoint& pt);
  };

  template <class baseDriver>
  class AbstractDriverButton : public AbstractButton
  {
  protected: 
    baseDriver* m_driver;
  public:
    AbstractDriverButton(const char* title) : AbstractButton(title){}
    
    baseDriver* SetDriver(baseDriver* driver)
    {
      return m_driver = driver;
    }
    
    virtual void OnClick() = 0;
  };
}

#endif /* ABSTRACTBUTTON_H */
