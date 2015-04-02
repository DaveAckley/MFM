#include "AbstractButton.h"
#include "SDL.h"
#include "Drawing.h"

namespace MFM
{
  void AbstractButton::Init()
  {
    SetEnabledForeground(GetForeground());
    SetEnabledBackground(GetBackground());

    SetDisabledForeground(Drawing::InterpolateColors(m_enabledFg,m_enabledBg,60));
    SetDisabledBackground(Drawing::InterpolateColors(m_enabledFg,m_enabledBg,40));
  }

  AbstractButton::AbstractButton()
    : m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(const char* text)
    : Label(text)
    , m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(SDL_Surface* icon)
    : Label(icon)
    , m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(const char* text, SDL_Surface* icon)
    : Label(text, icon)
    , m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::~AbstractButton()
  { }

  void AbstractButton::PaintComponent(Drawing& d)
  {
    if(!PaintClickHighlight(d))
    {
      PaintComponentNonClick(d);
    }
  }

  void AbstractButton::PaintComponentNonClick(Drawing & drawing)
  {
    SetForeground(m_enabled ? GetEnabledForeground() : GetDisabledForeground() );
    SetBackground(m_enabled ? GetEnabledBackground() : GetDisabledBackground() );
    Super::PaintComponent(drawing);
  }
}
