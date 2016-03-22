#include "Panel.h"
#include "Drawing.h"
#include "FileByteSink.h"  /* For STDOUT */

namespace MFM {

  // Do the double dispatching
  bool MouseButtonEvent::Handle(Panel & panel) {
    return panel.Handle(*this);
  }

  bool MouseMotionEvent::Handle(Panel & panel) {
    return panel.Handle(*this);
  }

  bool Panel::Handle(MouseButtonEvent & event)
  {
    return false;
  }

  bool Panel::Handle(MouseMotionEvent & event)
  {
    return false;
  }

  Panel::Panel(u32 width, u32 height)
  {
    SetDimensions(width, height);
    SetDesiredSize(U32_MAX, U32_MAX);  // Wish for a lot by default.

    m_forward = m_backward = 0;
    m_parent = m_top = 0;

    m_fontAsset = FONT_ASSET_NONE;
    SetName(0);

    m_bgColor = Drawing::BLACK;
    m_bdColor = Drawing::GREY;
    m_fgColor = Drawing::YELLOW;

    m_focusedChild = NULL;

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
    sink.Printf("(%d,%d)%dx%d,bg:%08x,bd:%08x,fg:%08x",
            m_rect.GetX(),
            m_rect.GetY(),
            m_rect.GetWidth(),
            m_rect.GetHeight(),
            m_bgColor,
            m_bdColor,
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

  u32 Panel::GetChildCount() const
  {
    u32 count = 0;
    if (m_top) {
      Panel * p = m_top;
      do {
        p = p->m_forward;
        ++count;
      } while (p != m_top);
    }
    return count;
  }


  void Panel::PrintFullName(ByteSink & sink) const
  {
    if (m_parent)
    {
      m_parent->PrintFullName(sink);
      sink.Printf(".");
    }
    sink.Printf("%s",GetName());
  }

  Panel * Panel::DereferenceFullName(ByteSource & in)
  {
    PanelNameString pns;
    if (!ScanPanelName(in,pns) || !pns.Equals(this->GetName()))
      return 0;

    return this->DereferenceDescendants(in);
  }

  Panel * Panel::DereferenceDescendants(ByteSource & in)
  {
    if (in.Peek() != '.') return this;
    in.Read(); // discard .

    PanelNameString pns;
    if (!ScanPanelName(in,pns)) return 0;

    Panel * p = m_top;
    while (p) {
      if (pns.Equals(p->GetName()))
        return p->DereferenceDescendants(in);
      p = p->m_backward;
      if (p == m_top) p = 0;
    }
    return 0;
  }

  bool Panel::LoadDetails(const char * key, LineCountingByteSource & source)
  {
    if (!strcmp("loc",key))
    {
      RectSerializer rs(m_rect);
      return 1 == source.Scanf("%@", &rs);
    }

    if (!strcmp("doc",key))
    {
      m_doc.Reset();
      return source.ScanDoubleQuotedString(m_doc);
    }

    if (!strcmp("bdc",key)) return 1 == source.Scanf("%08x",&m_bdColor);
    if (!strcmp("bgc",key)) return 1 == source.Scanf("%08x",&m_bgColor);
    if (!strcmp("fgc",key)) return 1 == source.Scanf("%08x",&m_fgColor);

    if (!strcmp("fnt",key))
    {
      u32 tmp;
      if (1 != source.Scanf("%d",&tmp)) return false;
      if (tmp > FONT_ASSET_NONE) return false;
      m_fontAsset = (FontAsset) tmp;
      return true;
    }

    if (!strcmp("dsz",key))
    {
      UPointSerializer uptmp(m_desiredSize);
      return 1 == source.Scanf("%@",&uptmp);
    }

    if (!strcmp("dlc",key))
    {
      SPointSerializer sptmp(m_desiredLocation);
      return 1 == source.Scanf("%@",&sptmp);
    }

    if (!strcmp("vis",key)) return 1 == source.Scanf("%?d", sizeof m_visible, &m_visible);

    return false; // That's all we got.
  }

  void Panel::SaveDetails(ByteSink & sink) const
  {
    {
      Rect tmp(m_rect);
      RectSerializer rs(tmp);
      sink.Printf(" PP(loc=%@)\n", &rs);
    }
    sink.Printf(" PP(doc=");
    sink.PrintDoubleQuotedString(m_doc.GetZString());
    sink.Printf(")\n");
    {
      sink.Printf(" PP(bdc=%08x)\n",m_bdColor);
      sink.Printf(" PP(bgc=%08x)\n",m_bgColor);
      sink.Printf(" PP(fgc=%08x)\n",m_fgColor);
    }
    {
      sink.Printf(" PP(fnt=%d)\n",m_fontAsset);
    }
    {
      UPoint tsz(m_desiredSize);
      UPointSerializer tmp(tsz);
      sink.Printf(" PP(dsz=%@)\n",&tmp);
    }
    {
      SPoint tsz(m_desiredLocation);
      SPointSerializer tmp(tsz);
      sink.Printf(" PP(dlc=%@)\n",&tmp);
    }
    {
      sink.Printf(" PP(vis=%d)\n",m_visible);
    }
  }

  void Panel::SaveAll(ByteSink & sink) const
  {
    sink.Printf("PanelConfig(");
    PrintFullName(sink);
    sink.Printf(")\n");
    SaveDetails(sink);
    Panel * p = m_top;
    while (p) {
      p->SaveAll(sink);
      p = p->m_backward;
      if (p == m_top) p = 0;
    }
  }

  bool Panel::IsLegalPanelName(const char * name)
  {
    if (!name) FAIL(NULL_POINTER);
    CharBufferByteSource in(name, strlen(name));
    return ScanPanelName(in, DevNullByteSink);
  }

  bool Panel::ScanPanelName(ByteSource & in, ByteSink & out)
  {
    PanelNameString pns;
    in.Scanf("%[a-zA-Z]%[_a-zA-Z0-9]",&pns,&pns);
    if (pns.GetLength() == 0 || pns.HasOverflowed())
      return false;
    pns.AppendTo(out);
    return true;
  }

  void Panel::Insert(Panel * child, Panel * after)
  {
    if (!child) FAIL(NULL_POINTER);
    if (child->m_parent) FAIL(ILLEGAL_ARGUMENT);

    // Must have a legal name!
    if (!IsLegalPanelName(child->m_name.GetZString()))
      FAIL(BAD_NAME);

    // Must not duplicate any existing kid name!
    if (m_top)
    {
      Panel * p = m_top;
      do {
        p = p->m_forward;
        if (child->m_name.Equals(p->m_name))
          FAIL(DUPLICATE_ENTRY);
      } while (p != m_top);
    }

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

  TTF_Font* Panel::GetFontReal() const {
    return AssetManager::Get(m_fontAsset);
  }

  FontAsset Panel::GetFont() const {
    return m_fontAsset;
  }

  FontAsset  Panel::SetFont(FontAsset newFont) {
    FontAsset old = m_fontAsset;
    m_fontAsset = newFont;
    return old;
  }

  void Panel::SetDimensions(u32 width, u32 height)
  {
    m_rect.SetWidth(width);
    m_rect.SetHeight(height);
  }

  void Panel::SetDesiredSize(u32 width, u32 height)
  {
    m_desiredSize.Set(width, height);
  }

  const UPoint & Panel::GetDimensions() const
  {
    return m_rect.GetSize();
  }

  const UPoint & Panel::GetDesiredSize() const
  {
    return m_desiredSize;
  }

  const SPoint & Panel::GetRenderPoint() const
  {
    return m_rect.GetPosition();
  }

  void Panel::SetRenderPoint(const SPoint & renderPt)
  {
    m_rect.SetPosition(renderPt);
    m_desiredLocation.Set(renderPt.GetX(), renderPt.GetY()); // XXX Is this right?
  }

  SPoint Panel::GetAbsoluteLocation()
  {
    return m_rect.GetPosition() +
      (m_parent ? m_parent->GetAbsoluteLocation() : SPoint(0,0));
  }

  void Panel::PaintUpdateVisibility(Drawing & config)
  {
    // Overridable; does nothing by default
  }

  void Panel::Paint(Drawing & drawing)
  {
    PaintUpdateVisibility(drawing);

    if(m_visible)
    {
      Rect old, cur;
      drawing.GetWindow(old);
      drawing.TransformWindow(m_rect);
      drawing.GetWindow(cur);

      FontAsset oldFont = FONT_ASSET_NONE;
      FontAsset font = GetFont();
      if (font != FONT_ASSET_NONE)
      {
        oldFont = drawing.SetFont(font);
      }

      const u32 oldfg = drawing.GetForeground();
      const u32 oldbg = drawing.GetForeground();
      drawing.SetForeground(m_fgColor);
      drawing.SetBackground(m_bgColor);

      PaintComponent(drawing);
      PaintBorder(drawing);

      drawing.SetWindow(cur);
      PaintChildren(drawing);

      drawing.SetWindow(cur);
      PaintFloat(drawing);

      drawing.SetWindow(old);

      if (oldFont)
        drawing.SetFont(oldFont);

      drawing.SetForeground(oldfg);
      drawing.SetBackground(oldbg);
    }
  }

  void Panel::PaintFloat(Drawing & drawing)
  {
    /* Overridable, empty by default */
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
    drawing.Clear();
  }

  void Panel::PaintBorder(Drawing & drawing)
  {
    drawing.SetForeground(m_bdColor);
    drawing.DrawRectangle(Rect(SPoint(),m_rect.GetSize()));
  }

  void Panel::SetAnchor(const GUIAnchor anchor)
  {
    switch(anchor)
    {
    case ANCHOR_NORTH:
      m_desiredLocation.SetY(0);
      break;
    case ANCHOR_EAST:
      /* This ought to be big enough */
      m_desiredLocation.SetX(1000000);
      break;
    case ANCHOR_SOUTH:
      m_desiredLocation.SetY(1000000);
      break;
    case ANCHOR_WEST:
      m_desiredLocation.SetX(0);
      break;
    default:
      break;
    }
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

  bool Panel::Dispatch(MouseEvent & event, const Rect & existing)
  {

    // Can't take events if we're not here.
    if (!m_visible)
      return false;

    SPoint at = event.GetAt();

    Rect newRect;
    Drawing::TransformWindow(existing, m_rect, newRect);

    if (!newRect.Contains(at))
      return false;

    if (m_top)
    {

      // Scan kids from top down so more visible gets first crack
      Panel * p = m_top;
      Panel* oldFocus = m_focusedChild;
      Panel* newFocus = NULL;
      bool handled = false;
      do
      {
        if (p->Dispatch(event, newRect))
        {
          newFocus = p;
          handled = true;
          break;
        }
        p = p->m_backward;
      } while (p != m_top);

      if(newFocus != oldFocus)
      {
        if (oldFocus)
        {
          oldFocus->OnMouseExit();
        }
        if (newFocus)
        {
          newFocus->OnMouseEnter();
        }
      }
      m_focusedChild = newFocus;

      if(handled)
      {
        return true;
      }
    }

    // Here the hit is in us and none of our descendants wanted it.
    // So it's ours if we do.
    return event.Handle(*this);
  }

  SPoint Panel::GetTextSize(FontAsset font, const char * text)
  {
    return MakeSigned(AssetManager::GetFontTextSize(font,text));
  }
} /* namespace MFM */
