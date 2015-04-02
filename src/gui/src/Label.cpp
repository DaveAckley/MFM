#include "Label.h"
#include "SDL.h"
#include "Drawing.h"

namespace MFM
{
  void Label::Init()
  {
    this->SetForeground(LABEL_COLOR);
    this->SetBorder(LABEL_BORDER_COLOR);
    this->SetBackground(LABEL_BACKGROUND_COLOR);

    m_text.Reset();
  }

  Label::Label()
    : m_icon(0)
  {
    Init();
  }

  Label::Label(const char* text)
    : m_icon(0)
  {
    Init();
    SetText(text);
  }

  Label::Label(SDL_Surface* icon)
    : m_icon(icon)
  {
    Init();
  }

  Label::Label(const char* text, SDL_Surface* icon)
    : m_icon(icon)
  {
    Init();
    SetText(text);
  }

  Label::~Label()
  { }

  void Label::PaintComponent(Drawing& drawing)
  {
    drawing.SetForeground(GetForeground());
    drawing.SetBackground(GetBackground());
    drawing.Clear();

    if(m_icon)
    {
      drawing.BlitImage(m_icon, UPoint(0,0), Panel::GetDimensions());
    }

    if(m_text.GetLength() > 0)
    {
      const char * zstr = m_text.GetZString();
      SPoint dims = MakeSigned(Panel::GetDimensions());
      SPoint textSize = Panel::GetTextSize(drawing.GetFont(), zstr);
      SPoint renderAt = max((dims-textSize)/2, SPoint(0,0));

      drawing.BlitText(zstr, MakeUnsigned(renderAt), GetDimensions());
    }
  }

}
