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
#include "Panel.h"
#include "LineTailByteSink.h"

namespace MFM {
  /**
   * A template class for a panel capable of holding a given number of
   * lines of text each no longer than a given number of columns.
   */
  template <u32 COLUMNS, u32 LINES>
  class TextPanel : public Panel
  {
  private: typedef Panel Super;

  public:
    TextPanel()
    {
      SetName("Text Panel");
      SetRenderPoint(SPoint(0, 0));
      SetForeground(Drawing::GREEN);
      SetBackground(Drawing::GREY05);
      SetBorder(Drawing::GREY20);

      m_bottomLineShown = 0;
    }

    ByteSink & GetByteSink() {
      return m_text;
    }

  protected:
    virtual void PaintComponent(Drawing& drawing)
    {
      Super::PaintComponent(drawing);
      RenderText(drawing);
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;
      if(event.type == SDL_MOUSEBUTTONDOWN) {

        SPoint pt = GetAbsoluteLocation();
        pt.Set(event.x - pt.GetX(),
               event.y - pt.GetY());

        switch (event.button) {

        case SDL_BUTTON_LEFT:
          if (pt.GetX() < (s32) ELEVATOR_WIDTH) {
            m_leftButtonDragStart = SPoint(event.x,event.y);//pt;
            m_dragStartElevatorBottom = m_elevatorBottom;
          }
          break;

        case SDL_BUTTON_WHEELUP:
        case SDL_BUTTON_WHEELDOWN:
          s32 dir = event.button == SDL_BUTTON_WHEELDOWN ? -1 : 1;
          if (mbe.m_keyboard.ShiftHeld()) dir *= 10;

          m_bottomLineShown = MAX<s32>(0, MIN<s32>(m_bottomLineShown + dir, m_text.GetLines()));
          break;
        }
      }

      return true;
    }

    virtual bool Handle(MouseMotionEvent& mbe)
    {
      SDL_MouseMotionEvent & event = mbe.m_event.motion;

      SPoint pt = GetAbsoluteLocation();
      pt.Set(event.x - pt.GetX(),
             event.y - pt.GetY());

      if ((pt.GetX() < (s32) ELEVATOR_WIDTH) &&
          (mbe.m_buttonMask & (1 << SDL_BUTTON_LEFT)) !=0) {
        SPoint nowAt(event.x, event.y);
        s32 deltay = nowAt.GetY() - m_leftButtonDragStart.GetY();
        m_bottomLineShown = ElevatorBottomToBottomLine(m_dragStartElevatorBottom + deltay);
      }

      // Whether we did anything or not, we're eating the move here
      return true;
    }

  private:
    static const u32 ELEVATOR_WIDTH = 12;
    static const u32 ELEVATOR_COLOR = 0xff007f00;

    LineTailByteSink<LINES,COLUMNS> m_text;
    u32 m_bottomLineShown;
    u32 m_lastBytesWritten;

    SPoint m_leftButtonDragStart;
    u32 m_dragStartElevatorBottom;

    u32 m_panelHeight;
    u32 m_fontHeight;
    u32 m_neededHeight;
    u32 m_textLines;
    u32 m_elevatorRange;
    u32 m_elevatorBottom;
    u32 m_elevatorHeight;

    // If we move elevatorBottom this many pixels, how many bottom lines
    // is that, based on the most-recent prior render?
    s32 ElevatorBottomToBottomLine(s32 newElevatorBottom) {
      // Don't wrap
      newElevatorBottom = MAX<s32>(0, MIN<s32>(newElevatorBottom, m_panelHeight - m_elevatorHeight));

      s32 newBot = m_textLines - newElevatorBottom * m_textLines / m_elevatorRange;
      return MAX<s32>(0, MIN<s32>(newBot, m_text.GetLines()));
    }

    void RenderText(Drawing & drawing)
    {
      m_panelHeight = GetHeight();
      if (m_panelHeight == 0)
        m_panelHeight = 1;  // WTF:(

      TTF_Font * font = drawing.GetFont();
      if (!font) return;  // WTF?

      m_fontHeight = TTF_FontLineSkip(font);
      if (m_fontHeight == 0)
        m_fontHeight = 1;   // WTF!

      // Figure out size of elevator
      m_textLines = m_text.GetLines();
      m_neededHeight = m_textLines * m_fontHeight;

      m_elevatorHeight = m_panelHeight;
      if (m_neededHeight > m_panelHeight)
        m_elevatorHeight = m_panelHeight * m_panelHeight / m_neededHeight;

      // Figure out elevator position
      if (m_lastBytesWritten != m_text.GetBytesWritten()) {

        // Snap to bottom on new output
        m_lastBytesWritten = m_text.GetBytesWritten();
        m_bottomLineShown = 0;
      }

      m_elevatorRange = m_panelHeight - m_elevatorHeight;
      m_elevatorBottom = m_elevatorRange * (m_textLines - m_bottomLineShown) / m_textLines;

      drawing.FillRect(1, m_elevatorBottom, ELEVATOR_WIDTH-2, m_elevatorHeight, ELEVATOR_COLOR);

      s32 y = m_panelHeight;
      for (s32 line = m_textLines - m_bottomLineShown - 1; line >= 0; --line) {
        const char * zline = m_text.GetZString(line);
        if (!zline) {
          printf("WTF %d\n", line);
        }
        drawing.BlitText(zline, UPoint(ELEVATOR_WIDTH, y), UPoint(GetWidth(), m_fontHeight));
        y -= m_fontHeight;
        if (y < 0) break;
      }
    }

  };

} /* namespace MFM */

#endif /* TEXTPANEL_H */
