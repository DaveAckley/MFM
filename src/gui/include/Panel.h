/*                                              -*- mode:C++ -*-
  Panel.h Hierarchical rendering system
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Panel.h Hierarchical rendering system
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PANEL_H
#define PANEL_H

#include "itype.h"
#include "Point.h"
#include "Rect.h"
#include "Drawing.h"
#include "ByteSink.h"
#include "EditingTool.h"
#include "OverflowableCharBufferByteSink.h"
#include "AssetManager.h"

namespace MFM
{

  typedef enum
  {
    ANCHOR_NORTH,
    ANCHOR_EAST,
    ANCHOR_SOUTH,
    ANCHOR_WEST
  }GUIAnchor;

  class Panel;   // FORWARD

  struct MouseEvent
  {
    MouseEvent(const u32 & keyboardModifiers, SDL_Event & event)
      : m_keyboardModifiers(keyboardModifiers)
      , m_event(event)
    { }

    const u32 & m_keyboardModifiers;
    SDL_Event & m_event;

    virtual ~MouseEvent()
    { }

    // Double dispatch support
    virtual bool Handle(Panel & panel) = 0;

    virtual SPoint GetAt() const = 0;
  };

  struct MouseButtonEvent : public MouseEvent
  {
    MouseButtonEvent(const u32 & keyboardModifiers, SDL_Event & event) :
      MouseEvent(keyboardModifiers, event)
    { }

    virtual bool Handle(Panel & panel) ;

    virtual SPoint GetAt() const
    {
      return SPoint(m_event.button.x, m_event.button.y);
    }
  };

  typedef SPoint ButtonPositionArray[SDL_BUTTON_X2+1];

  struct MouseMotionEvent : public MouseEvent
  {
    const u32 m_buttonMask;
    const ButtonPositionArray & m_buttonPositionArray;

    MouseMotionEvent(const u32 & keyboardModifiers, SDL_Event & event, u32 buttonMask,
                     ButtonPositionArray & bpa)
      : MouseEvent(keyboardModifiers, event)
      , m_buttonMask(buttonMask)
      , m_buttonPositionArray(bpa)
    { }

    virtual bool Handle(Panel & panel) ;

    virtual SPoint GetAt() const
    {
      return SPoint(m_event.motion.x, m_event.motion.y);
    }
  };

  /**
   * The base class for a region of the GUI system used in the MFM
   * simulator.
   */
  class Panel
  {
  private:
    typedef OString32 PanelNameString;
    PanelNameString m_name;

    typedef OString64 PanelDocString;
    PanelDocString m_doc; //< Used as help text and tool tip

    Rect m_rect;    // Size and location of panel relative to parent
    u32 m_bdColor;  // Default border color of this panel
    u32 m_bgColor;  // Default background color of this panel
    u32 m_fgColor;  // Default foreground color of this panel

    /**
     * Preferred font for text operations, if any, specified as an
     * AssetManager::FontAsset
     */
    FontAsset m_fontAsset;

    /**
     * The size that this Panel wants to be, when given enough room.
     */
    UPoint m_desiredSize;
    SPoint m_desiredLocation;

    // My parent Panel, if any
    Panel * m_parent;

    // The top of the stack of panels I am parent to.  This is
    // null when I have no kids
    Panel * m_top;

    // My position in my parent's doubly-linked circle of panels.
    // These are null when I have no parent.
    Panel * m_forward;    // Pointer to next guy above me in rendering order
    Panel * m_backward;   // Pointer to next guy below me in rendering order

    Panel* m_focusedChild;

    /**
     * Whether this Panel is currently being displayed, and
     * analogously, whether it is doing #Dispatch processing.
     */
    bool m_visible;

    // A helper function to indent a line, that totally belongs elsewhere
    static void Indent(ByteSink& sink, u32 count) ;

  public:
    void SetDoc(const char * doc)
    {
      MFM_API_ASSERT_NONNULL(doc);
      m_doc.Reset();
      m_doc.Printf("%s", doc);
    }

    const char * GetDoc() const
    {
      return m_doc.GetZString();
    }

    void SaveAll(ByteSink& to) const ;

    static bool IsLegalPanelName(const char * name) ;

    static bool ScanPanelName(ByteSource & in, ByteSink & out) ;

    Panel(u32 width=0, u32 height=0);

    virtual ~Panel();

    void Insert(Panel* child, Panel* afterOrNull) ;

    Panel * GetParent() { return m_parent; }

    Panel * GetTop() { return m_top; }
    //    Panel* Pop() ;

    u32 GetChildCount() const ;

    void Remove(Panel* child) ;

    void SetVisible(bool value){ m_visible = value; }
    bool IsVisible() const { return m_visible; }

    u32 GetWidth() const {return m_rect.GetWidth();}

    u32 GetHeight() const {return m_rect.GetHeight();}

    void SetDimensions(u32 width, u32 height);

    void SetDesiredSize(u32 width, u32 height);

    const UPoint & GetDimensions() const ;

    const UPoint & GetDesiredSize() const ;

    void SetRenderPoint(const SPoint & renderPt);

    const SPoint & GetRenderPoint() const ;

    const char * GetName() const { return m_name.GetBuffer(); }

    void SetName(const char * name)
    {
      m_name.Reset();
      if (name)
      {
        m_name.Print(name);
      }
      m_name.GetZString(); // Ensure null terminated to use GetBuffer()
    }

    /**
       Primarily for debugging, print a text rendering of this panel
       and its children.
     */
    void Print(ByteSink & sink, u32 indent = 0) const;

    /**
       Print the full name (i.e., including all its ancestors) of this
       panel.
     */
    void PrintFullName(ByteSink & sink) const ;

    /**
       Find the panel referred to by the name(s) on in.  If in does
       not begin with the name of this Panel, return null.  If it
       does, return the Panel returned by DereferenceDescendants, if
       any.
     */
    Panel * DereferenceFullName(ByteSource & in) ;

    /**
       Find the panel referred to by the name(s) on in.  If in begins
       with a '.', there is at least one more level to be dereferenced
       in the children of this.  If in does not begin with a '.', this
       is the answer.  Return null if not found
     */
    Panel * DereferenceDescendants(ByteSource & in) ;

    virtual void SaveDetails(ByteSink & sink) const ;

    virtual bool LoadDetails(const char * key, LineCountingByteSource & source) ;

    /**
       Get the current background color.
     */
    u32 GetBackground() const
    {
      return m_bgColor;
    }

    /**
       Set the background color.  Returns prior value
     */
    u32 SetBackground(const u32 color)
    {
      u32 old = m_bgColor;
      m_bgColor = color;
      return old;
    }

    /**
       Get the current border color.
     */
    u32 GetBorder() const
    {
      return m_bdColor;
    }

    /**
       Set the border color.  Returns prior value
     */
    u32 SetBorder(const u32 color)
    {
      u32 old = m_bdColor;
      m_bdColor = color;
      return old;
    }

    /**
       Get the current foreground color.
     */
    u32 GetForeground() const
    {
      return m_fgColor;
    }

    /**
       Set the foreground color.  Returns prior value
     */
    u32 SetForeground(const u32 color)
    {
      u32 old = m_fgColor;
      m_fgColor = color;
      return old;
    }

    /**
       Get the default font for this panel.  May return null.  Text
       drawing operations will use the prevailing font
       (Drawing::GetFont()) if this returns null.
    */
    TTF_Font* GetFontReal() const ;

    FontAsset GetFont() const ;

    /**
       Set the default font for this panel, specified as an FontAsset.
       FONT_ASSET_NONE may be passed in to clear the default font.
       Returns prior value.
    */
    FontAsset SetFont(FontAsset newFont) ;

    /**
     * Gets the absolute location of this Panel from the window that
     * the program is contained in.
     *
     * @returns The absolute location of this Panel from the window that
     *          the program is contained in.
     */
    SPoint GetAbsoluteLocation();

    bool Contains(SPoint& pt)
    {
      return m_rect.Contains(pt);
    }

    virtual void Paint(Drawing & config);

    /**
       Update the visibility of this Panel, if desired, at the
       beginning of the painting process.  The default implementation
       (which may be accessed, if overridden, via
       this->Panel::IsVisible) does nothing; overriding
       implementations may use SetVisible to alter the visibility of
       this Panel before its painting begins in earnest.  Note this
       method is called only during painting; during dispatching the
       m_visible variable is checked directly.

       @sa SetVisible

     */
    virtual void PaintUpdateVisibility(Drawing & config);

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
       Paint material, if any, that is supposed to 'float' over the
       component and its children.  The default implementation paints
       nothing. 
     */
    virtual void PaintFloat(Drawing & config);

    /**
       Dispatch a mouse event, to the appropriate subpanel depending on
       the stacking order and the position of the mouse.  Panels should
       override the Handle(MouseButtonEvent) and/or
       Handle(MouseMotionEvent) method if they wish to handle such
       events.  Returns true if any panel claimed to handle the event.
     */
    virtual bool Dispatch(MouseEvent & event, const Rect & rect);

    /**
       Respond to a MouseButtonEvent (this includes scroll wheel
       events).

       @returns true if the event should be considered handled.  The
                default implementation does nothing and returns
                false.
     */
    virtual bool Handle(MouseButtonEvent & event) ;

    /**
       Respond to a MouseMotionEvent.  Return true if the event should
       be considered handled.  The default implementation does nothing
       and returns false.
     */
    virtual bool Handle(MouseMotionEvent & event) ;

    /**
       Called when a MouseMotionEvent happens which is equivalent to
       the mouse exiting this Panel . This is, for instance, useful if
       a Panel needs to stop following the mouse.  By default, does
       nothing.
    */
    virtual void OnMouseExit()
    {
      /* No behavior by default */
    }

    /**
       Called when a MouseMotionEvent happens which is equivalent to
       the mouse entering this Panel . By default, does nothing.
    */
    virtual void OnMouseEnter()
    {
      /* No behavior by default */
    }

    /**
     * Respond to the resizing of this panel's parent.
     */
    virtual void HandleResize(const UPoint& parentSize);

    /**
     * Used to tell this Panel to stick to a particular edge of its
     * parent based on a specified GUIAnchor.
     *
     * @param anchor The GUIAnchor which specifies the edge of this
     *               Panel's bounds to stick to.
     */
    void SetAnchor(const GUIAnchor anchor);

    /**
     * Calculates the dimensions of the resulting SDL_Surface
     * generated when rendering a given string using a given TTF_Font .
     *
     * @param font The TTF_Font used to calculate the resulting size
     *             of the final SDL_Surface .
     *
     * @param text The string used to calculate the resulting size of
     *             the final SDL_Surface .
     *
     * @returns A SPoint representing the size of the resulting
     *          SDL_Surface generated by rendering \c text in \c font
     *          font.
     */
    SPoint GetTextSize(FontAsset font, const char * text);
    //    SPoint GetTextSize(TTF_Font * font, const char * text);

  };
} /* namespace MFM */
#endif /*PANEL_H*/
