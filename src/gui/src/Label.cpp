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
    this->SetFont(FONT_ASSET_LABEL);

    m_text.Reset();
  }

  Label::Label()
    : m_iconAsset(IMAGE_ASSET_NONE)
  {
    Init();
  }

  Label::Label(const char* text)
    : m_iconAsset(IMAGE_ASSET_NONE)
  {
    Init();
    SetText(text);
  }

  Label::Label(ImageAsset icon)
    : m_iconAsset(icon)
  {
    Init();
  }

  Label::Label(const char* text, ImageAsset icon)
    : m_iconAsset(icon)
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

    // Does nothing if IMAGE_ASSET_NONE
    drawing.BlitImageAsset(m_iconAsset, UPoint(0,0));

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
