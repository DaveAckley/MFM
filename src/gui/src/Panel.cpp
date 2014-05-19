#include "Panel.h"
#include "Drawing.h"
#include "FileByteSink.h"  /* For STDOUT */

namespace MFM {

  Panel::Panel(u32 width, u32 height)
  {
    SetDimensions(width, height);

    m_forward = m_backward = 0;
    m_parent = m_top = 0;

    m_name = 0;

    m_bgColor = Drawing::BLACK;
    m_fgColor = Drawing::YELLOW;

    m_visible = true;
  }

  Panel::~Panel()
  {

    // Eject any content in us
    while (m_top)
      Remove(m_top);

    // Eject us from our parent
    if (m_parent)
      m_parent->Remove(this);
  }

  void Panel::Indent(ByteSink & sink, u32 count)
  {
    for (u32 i = 0; i < count; ++i)
      sink.WriteByte(' ');
  }
  void Panel::Print(ByteSink & sink, u32 indent) const
  {
    Indent(sink,indent);
    sink.Printf("[");
    if (GetName()) sink.Printf("%s:", GetName());
    sink.Printf("%p",(void*) this);
    sink.Printf("(%d,%d)%dx%d,bg:%08x,fg:%08x",
            m_rect.GetX(),
            m_rect.GetY(),
            m_rect.GetWidth(),
            m_rect.GetHeight(),
            m_bgColor,
            m_fgColor);
    if (m_top) {
      Panel * p = m_top;
      sink.Printf("\n");
      do {
        p = p->m_forward;
        p->Print(sink, indent+2);
      } while (p != m_top);
      Indent(sink,indent);
    }
    sink.Printf("]\n");
  }

  void Panel::Insert(Panel * child, Panel * after)
  {
    if (!child) FAIL(NULL_POINTER);
    if (child->m_parent) FAIL(ILLEGAL_ARGUMENT);

    if (!m_top) {

      if (after) FAIL(ILLEGAL_ARGUMENT);
      m_top = child->m_forward = child->m_backward = child;

    } else {

      if (!after) after = m_top;
      else if (after->m_parent != this) FAIL(ILLEGAL_ARGUMENT);

      child->m_forward = after->m_forward;
      child->m_backward = after;
      after->m_forward->m_backward = child;
      after->m_forward = child;
    }

    child->m_parent = this;
  }

  void Panel::Remove(Panel * child)
  {
    if (!child) FAIL(NULL_POINTER);
    if (child->m_parent != this) FAIL(ILLEGAL_ARGUMENT);

    if (child->m_forward == child)  // Single elt list
      m_top = 0;
    else {
      if (m_top == child)
        m_top = child->m_forward;
      child->m_forward->m_backward = child->m_backward;
      child->m_backward->m_forward = child->m_forward;
    }
    child->m_parent = 0;
    child->m_forward = 0;
    child->m_backward = 0;
  }

  void Panel::SetDimensions(u32 width, u32 height)
  {
    m_rect.SetWidth(width);
    m_rect.SetHeight(height);
    m_desiredSize.Set(width, height);
  }

  const UPoint & Panel::GetDimensions() const
  {
    return m_rect.GetSize();
  }

  void Panel::SetRenderPoint(const SPoint & renderPt)
  {
    m_rect.SetPosition(renderPt);
    m_desiredLocation.Set(renderPt.GetX(), renderPt.GetY());
  }

  SPoint Panel::GetAbsoluteLocation()
  {
    return m_rect.GetPosition() +
      (m_parent ? m_parent->GetAbsoluteLocation() : SPoint(0,0));
  }

  void Panel::Paint(Drawing & drawing)
  {
    if(m_visible)
    {
      Rect old, cur;
      drawing.GetWindow(old);
      drawing.TransformWindow(m_rect);
      drawing.GetWindow(cur);

      PaintComponent(drawing);
      PaintBorder(drawing);

      drawing.SetWindow(cur);
      PaintChildren(drawing);

      drawing.SetWindow(old);
    }
  }

  void Panel::PaintChildren(Drawing & drawing)
  {
    if (m_top) {
      Rect cur;
      drawing.GetWindow(cur);

      Panel * p = m_top;
      do {
        p = p->m_forward;
        drawing.SetWindow(cur);
        p->Paint(drawing);
      } while (p != m_top);
    }
  }

  void Panel::PaintComponent(Drawing & drawing)
  {
    drawing.SetForeground(m_fgColor);
    drawing.SetBackground(m_bgColor);
    drawing.Clear();
  }

  void Panel::PaintBorder(Drawing & drawing)
  {
    drawing.SetForeground(m_fgColor);
    drawing.DrawRectangle(Rect(SPoint(),m_rect.GetSize()));
  }

  void Panel::HandleResize(const UPoint& parentSize)
  {
    /* Try to make myself as big as I can, then call on my children. */

    if(m_desiredSize.GetX() > parentSize.GetX())
    {
      m_rect.SetX(0);
      m_rect.SetWidth(parentSize.GetX());
    }
    else
    {
      m_rect.SetX(MIN<u32>(m_desiredLocation.GetX(),
			   parentSize.GetX() - m_rect.GetWidth()));
      m_rect.SetWidth(m_desiredSize.GetX());
    }


    if(m_desiredSize.GetY() > parentSize.GetY())
    {
      m_rect.SetY(0);
      m_rect.SetHeight(parentSize.GetY());
    }
    else
    {
      m_rect.SetY(MIN<u32>(m_desiredLocation.GetY(),
			   parentSize.GetY() - m_rect.GetHeight()));
      m_rect.SetHeight(m_desiredSize.GetY());
    }

    if (m_top)
    {
      Rect cur;
      Panel * p = m_top;
      do
      {
        p = p->m_forward;
        p->HandleResize(m_rect.GetSize());
      } while (p != m_top);
    }

  }

  bool Panel::Dispatch(SDL_Event & event, const Rect & existing)
  {
    SPoint at;
    SDL_MouseButtonEvent * button = 0;
    SDL_MouseMotionEvent * motion = 0;

    switch (event.type) {
    case SDL_MOUSEMOTION:
      motion = &event.motion;
      at.SetX(motion->x);
      at.SetY(motion->y);
      break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
      button = &event.button;
      at.SetX(button->x);
      at.SetY(button->y);
      break;
    default:
      return false;
    }

    Rect newRect;
    Drawing::TransformWindow(existing, m_rect, newRect);

    if (!newRect.Contains(at))
      return false;

    if (m_top) {

      // Scan kids from top down so more visible gets first crack
      Panel * p = m_top;
      do {
        if (p->Dispatch(event, newRect))
          return true;
        p = p->m_backward;
      } while (p != m_top);
    }

    // Here the hit is in us and none of our descendants wanted it.
    // So it's ours if we do.

    switch (event.type) {
    case SDL_MOUSEMOTION:
      return Handle(*motion);
    case SDL_MOUSEBUTTONUP:
      printf("Panel Button Up");
      Print(STDOUT);
      // FALL THROUGH
    case SDL_MOUSEBUTTONDOWN:
      return Handle(*button);
    default:
      break;
    }

    return false;
  }

} /* namespace MFM */
