#include "V6Panel.h"

namespace MFM {

  bool V6Panel::Handle(KeyboardEvent & event) {
    if (this->Super::Handle(event)) return true;
    LOG.Warning("Handle(KeyboardEvent & event)");
    return false;
  }

  bool V6Panel::Handle(MouseButtonEvent & event) {
    if (this->Super::Handle(event)) return true;
    LOG.Warning("Handle(MouseButtonEvent & event)");
    return false;
  }

  bool V6Panel::Handle(MouseMotionEvent & event) {
    if (this->Super::Handle(event)) return true;
    LOG.Warning("Handle(MouseMotionEvent & event)");
    return false;
  }

  void V6Panel::Init(const char * owner, unsigned i) 
  {
    OString32 name;
    name.Printf("V6Panel-%s-%D",owner,i);
    SetName(name.GetZString());
    SetVisible(false);
    SetRenderPoint(SPoint(300+20*i,30+20*i));
    Panel::SetDesiredSize(400,100);
  }

  void V6Panel::PaintBorder(Drawing & d)
  {
    u32 oldBd = GetBorder();
    if (true) //GetGridPanel().IsSelectedAtomViewPanel(*this))
      SetBorder(Drawing::BLUE);
    
    this->Super::PaintBorder(d);
    SetBorder(oldBd);
  }

  void V6Panel::PaintComponent(Drawing& d)
  {
    this->Super::PaintComponent(d);
    if (true) {
      u32 oldfg = d.GetForeground();
      d.SetForeground(cBG_COLOR);
      d.FillRect(Rect(SPoint(0, 0), Panel::GetDimensions()));
      d.SetForeground(oldfg);
    }

    if (true) {
      u32 baseX = 20;
      u32 currentY = 5;
      const char * str = GetName();
      const UPoint up = Panel::GetDimensions();

      u32 tsize = up.GetX()/20; // vary text size with window size?
      tsize = MAX(tsize,5u);
      tsize = MIN(tsize,200u);
      TTF_Font * ttf = AssetManager::GetZFont(1, tsize);
      d.SetForeground(Drawing::WHITE);
      d.SetBackground(Drawing::BLACK);
      d.BlitText(str, SPoint(baseX + 20, currentY), MakeUnsigned(d.GetTextSize(str)),ttf);
    }

    //    LOG.Warning("v6p:pc");
  }

}
