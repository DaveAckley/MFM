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
    , m_iconPosition(0,0)
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

    SPoint dims = MakeSigned(Panel::GetDimensions());

    s32 textX = -1;
    
    if (m_zIconAsset.HasIcon())
    {
      u32 height = dims.GetY();  // Draw at (up to) 100% of label height, centered on icon position
      drawing.BlitIconAsset(m_zIconAsset, height, m_iconPosition);

      // Start text next to icon with constant plus proportional padding
      textX = m_iconPosition.GetX() + height + 1 + height/25;
    }
    else 
    {
      // Does nothing if IMAGE_ASSET_NONE
      drawing.BlitImageAsset(m_iconAsset, m_iconPosition);
    }

    if(m_text.GetLength() > 0)
    {
      const char * zstr = m_text.GetZString();
      SPoint textSize = Panel::GetTextSize(drawing.GetFont(), zstr);
      SPoint renderAt = max((dims-textSize)/2, SPoint(0,0));
      if (textX >= 0)
        renderAt.SetX(textX);
      drawing.BlitText(zstr, renderAt, GetDimensions());
    }
  }

}
