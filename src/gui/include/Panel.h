#ifndef PANEL_H     /* -*- C++ -*- */
#define PANEL_H

#include "itype.h"
#include "Point.h"
#include "Rect.h"
#include "Drawing.h"
#include <stdio.h>      /* For FILE */

namespace MFM {

  class Panel
  {
  protected:

    const char * m_name;

    Rect m_rect;
    u32 m_bgColor;  // Default background color of this panel
    u32 m_fgColor;  // Default foreground color of this panel

    // My parent Panel, if any
    Panel * m_parent;

    // Which side(s), if any, that I stick to, when my parent resizes.
    // (0,0), no automatic repositioning on parent resize, otherwise
    // sgn(GetX()) -> Left, Center, Right, and sgn(GetY()) -> Top,
    // Middle, Bottom
    // XXX Future: SPoint m_gravity;

    // The top of the stack of panels I am parent to.  This is
    // null when I have no kids
    Panel * m_top;

    // My position in my parent's doubly-linked circle of panels.
    // These are null when I have no parent.
    Panel * m_forward;    // Pointer to next guy above me in rendering order
    Panel * m_backward;   // Pointer to next guy below me in rendering order

    bool m_visible;

    static void Indent(FILE * file, u32 count) ;

  public:

    Panel(u32 width=0, u32 height=0);

    virtual ~Panel();

    void Insert(Panel * child, Panel * afterOrNull) ;

    Panel * Pop() ;

    void Remove(Panel * child) ;

    void SetVisibility(bool value){ m_visible = value; }

    void ToggleVisibility(){ m_visible = !m_visible; }

    u32 GetWidth() {return m_rect.GetWidth();}

    u32 GetHeight() {return m_rect.GetHeight();}

    void SetDimensions(u32 width, u32 height);

    const UPoint & GetDimensions() const ;

    void SetRenderPoint(const SPoint & renderPt);

    const char * GetName() const { return m_name; }

    void SetName(const char * name) { m_name = name; }

    void Print(FILE * file, u32 indent = 0) const;

    /**
       Get the current background color.
     */
    u32 GetBackground() const {
      return m_bgColor;
    }

    /**
       Set the background color.  Returns prior value
     */
    u32 SetBackground(const u32 color) {
      u32 old = m_bgColor;
      m_bgColor = color;
      return old;
    }

    /**
       Get the current foreground color.
     */
    u32 GetForeground() const {
      return m_fgColor;
    }

    /**
       Set the foreground color.  Returns prior value
     */
    u32 SetForeground(const u32 color) {
      u32 old = m_fgColor;
      m_fgColor = color;
      return old;
    }

    /**
     * Gets the absolute location of this Panel from the window that
     * the program is contained in.
     *
     * @returns The absolute location of this Panel from the window that
     *          the program is contained in.
     */
    SPoint GetAbsoluteLocation();


    virtual void Paint(Drawing & config);

    /**
       Paint the component itself, excluding its border and any
       children it may have.  The default implementation (which may be
       accessed, if overridden, via this->Panel::PaintComponent) sets
       the drawing foreground and background colors to the panel
       colors, and Clear()s the Panel.
     */
    virtual void PaintComponent(Drawing & config);

    /**
       Paint the border of the Panel, if it has one.  The default
       implementation paints a one pixel foreground color border.
     */
    virtual void PaintBorder(Drawing & config);

    /**
       Paint the subpanels of the Panel, if it has any.  In general
       this should not be overridden.  The default implementation
       draws each child in turn from the bottom of the stacking order
       to the top.
     */
    virtual void PaintChildren(Drawing & config);

    /**
       Dispatch a mouse event to the appropriate subpanel depending on
       the stacking order and the position of the mouse.  Panels should
       override the Handle(SDL_MouseButtonEvent) and/or
       Handle(SDL_MouseMotionEvent) method if they wish to handle such
       events.  Returns true if any panel claimed to handle the event.
     */
    virtual bool Dispatch(SDL_Event & event, const Rect & rect);

    /**
       Respond to a MouseButtonEvent (this includes scroll wheel
       events).  Return true if the event should be considered
       handled.  The default implementation does nothing and returns
       false.
     */
    virtual bool Handle(SDL_MouseButtonEvent & event)
    {
      //printf("Mouse %d@(%d,%d)", event.type, event.x, event.y);
      return false;
    }

    /**
       Respond to a MouseMotionEvent.  Return true if the event should
       be considered handled.  The default implementation does nothing
       and returns false.
     */
    virtual bool Handle(SDL_MouseMotionEvent & event)
    {
      return false;
    }
  };
} /* namespace MFM */
#endif /*PANEL_H*/

