#include "AbstractButton.h"
#include "SDL.h"
#include "Drawing.h"

namespace MFM
{
  void AbstractButton::Init()
  {
    this->SetEnabledFg(Drawing::GREY80);
    this->SetEnabledBg(Drawing::GREY20);
    this->SetFont(FONT_ASSET_BUTTON);
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

  AbstractButton::AbstractButton(ImageAsset icon)
    : Label(icon)
    , m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::AbstractButton(const char* text, ImageAsset icon)
    : Label(text, icon)
    , m_enabled(true)
    , m_justClicked(false)
  {
    Init();
  }

  AbstractButton::~AbstractButton()
  { }

  void AbstractButton::PaintBorder(Drawing & drawing)
  {
    if (!m_enabled) return;

    const u32 top = m_justClicked ? Drawing::BLACK : Drawing::WHITE;
    const u32 bot = m_justClicked ? Drawing::WHITE : Drawing::BLACK;

    const u32 bd = Panel::GetBorder();
    const u32 PERCENT_ORIG = 80;
    const u32 topc = InterpolateColors(bd, top, PERCENT_ORIG);
    const u32 botc = InterpolateColors(bd, bot, PERCENT_ORIG);

    const u32 height = Panel::GetHeight();
    const u32 width = Panel::GetWidth();
    drawing.SetForeground(topc);
    drawing.DrawHLine(0, 1, width);
    drawing.DrawVLine(0, 1, height);

    drawing.SetForeground(botc);
    drawing.DrawHLine(height - 1, 1, width);
    drawing.DrawVLine(width - 1, 1, height);
  }


  void AbstractButton::PaintComponent(Drawing& d)
  {
    if(!PaintClickHighlight(d))
    {
      PaintComponentNonClick(d);
    }
  }

  void AbstractButton::SetColorsFromEnabling()
  {
    u32 fg = m_enabledFg;
    u32 bg = m_enabledBg;
    if (!m_enabled)
    {
      const u32 nfg = InterpolateColors(fg, bg, 80);
      bg = InterpolateColors(bg, fg, 80);
      fg = nfg;
    }
    Panel::SetForeground(fg);
    Panel::SetBackground(bg);
    Label::GetIconAsset().SetEnabled(m_enabled);
  }

  void AbstractButton::PaintComponentNonClick(Drawing & drawing)
  {
    SetColorsFromEnabling();
    Super::PaintComponent(drawing);
  }
}
