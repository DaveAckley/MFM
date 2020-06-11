/*                                              -*- mode:C++ -*-
  TextPanel.h Panel for text content
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
  \file TextPanel.h Panel for text content
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef TEXTPANEL_H
#define TEXTPANEL_H

#include "itype.h"
#include "MovablePanel.h"
#include "LineTailByteSink.h"

namespace MFM {
  class GenericTextPanel : public MovablePanel
  {
  private:
    typedef MovablePanel Super;

  public:
    GenericTextPanel()
      : Super()
      , m_bottomLineShown(0)
      , m_lastBytesWritten(0)
      , m_leftButtonDragStart(-1,-1)
      , m_dragStartElevatorBottom(0)
      , m_panelHeight(100)
      , m_fontHeight(10)
      , m_neededHeight(10)
      , m_linesInPanel(10)
      , m_textLines(1)
      , m_elevatorRange(1)
      , m_elevatorBottom(0)
      , m_elevatorHeight(10)
      , m_elevatorWidth(ELEVATOR_WIDTH_DEFAULT)
    {
      SetName("TextPanel");
      SetRenderPoint(SPoint(0, 0));
      SetForeground(Drawing::GREEN);
      SetBackground(Drawing::GREY05);
      SetBorder(Drawing::GREY20);

      m_bottomLineShown = 0;
    }

    virtual ResettableByteSink & GetByteSink() = 0;

  protected:
    static const u32 ELEVATOR_WIDTH_DEFAULT = 18;
    u32 m_bottomLineShown;
    u32 m_lastBytesWritten;

    SPoint m_leftButtonDragStart;
    u32 m_dragStartElevatorBottom;

    u32 m_panelHeight;
    u32 m_fontHeight;
    s32 m_fontHeightAdjust;
    u32 m_neededHeight;
    u32 m_linesInPanel;
    u32 m_textLines;
    u32 m_elevatorRange;
    u32 m_elevatorBottom;
    u32 m_elevatorHeight;
    u32 m_elevatorWidth;

  };

  /**
   * A template class for a panel capable of holding a given number of
   * lines of text each no longer than a given number of columns.
   */
  template <u32 COLUMNS, u32 LINES>
  class TextPanel : public GenericTextPanel
  {
  private:
    typedef GenericTextPanel Super;

  public:
    typedef LineTailByteSink<LINES,COLUMNS> TextPanelByteSink;

    TextPanel()
      : Super()
      , m_text()
    {
    }

    virtual ~TextPanel() { } //avoid inline error

    virtual ResettableByteSink & GetByteSink() {
      return m_text;
    }

  protected:

    virtual void SaveDetails(ByteSink& sink) const
    {
      Super::SaveDetails(sink);
      sink.Printf(" PP(blsn=%d)\n",m_bottomLineShown);
      sink.Printf(" PP(clrt=1)\n");
      for (u32 i = 0; i < m_text.GetLines(); ++i)
      {
        const char * z = m_text.GetZString(i);
        if (i == m_text.GetLines() - 1 && strlen(z) == 0)
          break;             // Skip last empty line.. (see below)
        sink.Printf(" PP(txln=");
        sink.PrintDoubleQuotedString(z);
        sink.Printf(")\n");
      }
      sink.Printf(" PP(snap=%d)\n",m_lastBytesWritten != m_text.GetBytesWritten());
    }

    virtual bool LoadDetails(const char * key, LineCountingByteSource& source)
    {
      if (Super::LoadDetails(key, source)) return true;

      if (!strcmp("blsn",key)) return 1 == source.Scanf("%d",&m_bottomLineShown);
      if (!strcmp("clrt",key) && 1 == source.Scanf("1"))
      {
        m_text.Reset();
        return true;
      }
      if (!strcmp("txln",key))
      {
        OString128 buf;
        if (!source.ScanDoubleQuotedString(buf)) return false;
        m_text.Printf("%s\n",buf.GetZString()); // (see above) ..it's implied by this \n
        return true;
      }
      if (!strcmp("snap",key))
      {
        u32 moreText;
        if (1 != source.Scanf("%d",&moreText)) return false;
        if (moreText)
          m_lastBytesWritten = 0;
        else
          m_lastBytesWritten = m_text.GetBytesWritten();
        return true;
      }
      return false;
    }

    void ScrollToTop()
    {
      m_bottomLineShown = m_textLines - m_linesInPanel;
    }

    virtual void PaintComponent(Drawing& drawing)
    {
      Super::PaintComponent(drawing);
      RenderText(drawing);
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      if(event.type == SDL_MOUSEBUTTONDOWN && mbe.m_keyboardModifiers == 0) {

        SPoint pt = GetAbsoluteLocation();
        pt.Set(event.x - pt.GetX(),
               event.y - pt.GetY());

        switch (event.button) {

        case SDL_BUTTON_LEFT:
          if (pt.GetX() < (s32) m_elevatorWidth) {
            m_leftButtonDragStart = SPoint(event.x,event.y);//pt;
            m_dragStartElevatorBottom = m_elevatorBottom;
          }
          break;

        case SDL_BUTTON_WHEELUP:
        case SDL_BUTTON_WHEELDOWN:
          s32 dir = event.button == SDL_BUTTON_WHEELDOWN ? -1 : 1;
          if (mbe.m_keyboardModifiers & KMOD_SHIFT) dir *= 10;

          m_bottomLineShown = MAX<s32>(0, MIN<s32>(m_bottomLineShown + dir, m_textLines - m_linesInPanel));
          break;
        }
        return true;
      }
      return Super::Handle(mbe);
    }

    virtual bool Handle(MouseMotionEvent& mbe)
    {
      SDL_MouseMotionEvent & event = mbe.m_event.motion;

      SPoint pt = GetAbsoluteLocation();
      pt.Set(event.x - pt.GetX(),
             event.y - pt.GetY());

      if ((pt.GetX() < (s32) m_elevatorWidth) &&
          (mbe.m_buttonMask & (1 << SDL_BUTTON_LEFT)) !=0) {
        SPoint nowAt(event.x, event.y);
        s32 deltay = nowAt.GetY() - m_leftButtonDragStart.GetY();
        m_bottomLineShown = ElevatorBottomToBottomLine(m_dragStartElevatorBottom + deltay);
        return true;
      }

      return Super::Handle(mbe);
    }

  private:
    static const u32 ELEVATOR_COLOR = 0xff007f00;

    TextPanelByteSink m_text;

    // If we move elevatorBottom this many pixels, how many bottom lines
    // is that, based on the most-recent prior render?
    s32 ElevatorBottomToBottomLine(s32 newElevatorBottom) {
      // Don't wrap
      newElevatorBottom = MAX<s32>(0, MIN<s32>(newElevatorBottom, m_panelHeight - m_elevatorHeight));

      s32 newBot = m_textLines - newElevatorBottom * m_textLines / m_elevatorRange;
      return MAX<s32>(0, MIN<s32>(newBot, m_textLines - m_linesInPanel));
    }

    void RenderText(Drawing & drawing)
    {
      drawing.SetFont(Panel::GetFont());
      m_panelHeight = GetHeight();
      if (m_panelHeight == 0)
        m_panelHeight = 1;  // WTF:(

      TTF_Font * font = AssetManager::Get(drawing.GetFont());
      if (!font) return;  // WTF?

      m_fontHeight = TTF_FontLineSkip(font);
      if (m_fontHeight == 0)
        m_fontHeight = 1;   // WTF!

      m_textLines = m_text.GetLines();
      m_linesInPanel = m_panelHeight / m_fontHeight;

      m_neededHeight = m_textLines * m_fontHeight;

      m_elevatorHeight = m_panelHeight;
      if (m_neededHeight > m_panelHeight)
      {
        m_elevatorHeight = m_panelHeight * m_linesInPanel / (m_textLines - m_linesInPanel);
      }

      // Figure out elevator position
      if (m_lastBytesWritten != m_text.GetBytesWritten()) {

        // Snap to bottom on new output
        m_lastBytesWritten = m_text.GetBytesWritten();
        m_bottomLineShown = 0;
      }

      if (m_panelHeight <= m_elevatorHeight)
        m_elevatorRange = 1;
      else
        m_elevatorRange = m_panelHeight - m_elevatorHeight;

      m_elevatorBottom = m_elevatorRange * (m_textLines - m_bottomLineShown) / m_textLines;

      drawing.FillRect(1, m_elevatorBottom, m_elevatorWidth-2, m_elevatorHeight, ELEVATOR_COLOR);

      s32 y = m_panelHeight;
      for (s32 line = m_textLines - m_bottomLineShown - 1; line >= 0; --line) {
        const char * zline = m_text.GetZString(line);
        if (!zline) {
          LOG.Error("WTF %d\n", line);
          break;
        }
        drawing.BlitText(zline,
                         SPoint(m_elevatorWidth, y),
                         UPoint(GetWidth(), m_fontHeight));
        y -= m_fontHeight;
        if (y < 0) break;
      }
    }

  };

} /* namespace MFM */

#endif /* TEXTPANEL_H */
