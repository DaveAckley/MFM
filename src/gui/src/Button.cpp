#include "Button.h"
#include "SDL/SDL.h"
#include "Drawing.h"

namespace MFM
{
  Button::Button()
  {
    m_func = BUTTONFUNC_NOTHING;
    m_text = NULL;
  }

  Button::Button(ButtonFunction func, const char* text, SPoint location, SPoint dimensions)
  {
    m_func = func;
    m_text = text;
    m_location = location;
    m_dimensions = dimensions;
  }

  Button::~Button()
  {}

  void Button::Render(SDL_Surface* sfc, SPoint& offset, TTF_Font* font)
  {
    SPoint loc = m_location + offset;
    UPoint floc = MakeUnsigned(loc);
    floc.SetX(floc.GetX() + 20);
    Drawing::FillRect(sfc, 
		      loc.GetX() - BUTTON_BORDER_SIZE, 
		      loc.GetY() - BUTTON_BORDER_SIZE, 
		      m_dimensions.GetX() + 2 * BUTTON_BORDER_SIZE, 
		      m_dimensions.GetY() + 2 * BUTTON_BORDER_SIZE,
		      BUTTON_BORDER_COLOR);
    Drawing::FillRect(sfc, 
		      loc.GetX(),
		      loc.GetY(),
		      m_dimensions.GetX(),
		      m_dimensions.GetY(),
		      BUTTON_COLOR);

    Drawing::BlitText(sfc, font, m_text, 
		      floc,
		      MakeUnsigned(m_dimensions), BUTTON_BORDER_COLOR);
  }

  ButtonFunction Button::Contains(SPoint& pt)
  {
    if(pt.GetX() > m_location.GetX() &&
       pt.GetY() > m_location.GetY() &&
       pt.GetX() < m_location.GetX() + m_dimensions.GetX() &&
       pt.GetY() < m_location.GetY() + m_dimensions.GetY())
    {
      return m_func;
    }
    
    return BUTTONFUNC_NOTHING;
  }
}
