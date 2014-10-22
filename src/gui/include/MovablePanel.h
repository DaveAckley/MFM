/*                                              -*- mode:C++ -*-
  MovablePanel.h Dragging layer for panels which wich to be dragged
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
  \file MovablePanel.h Dragging layer for panels which wich to be dragged
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef MOVABLEPANEL_H
#define MOVABLEPANEL_H

#include "Panel.h"

namespace MFM
{
  class MovablePanel : public Panel
  {
   private:
    SPoint m_dragPoint;
    SPoint m_preDragLocation;
    bool m_dragging;

   public:
    MovablePanel(u32 width = 0, u32 height = 0) :
      Panel(width, height),
      m_dragging(false)
    { }

    void BeginDrag(MouseButtonEvent& event)
    {
      m_dragging = true;
      m_dragPoint = event.GetAt();
      m_preDragLocation = Panel::GetRenderPoint();
    }

    void EndDrag()
    {
      m_dragging = false;
    }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.m_keyboard.CtrlHeld())
        {
          BeginDrag(event);
          return true;
        }
      }
      else if(m_dragging && event.m_event.type == SDL_MOUSEBUTTONUP)
      {
        EndDrag();
        return true;
      }

      return PostDragHandle(event);
    }

    virtual bool Handle(MouseMotionEvent& event)
    {
      if(m_dragging)
      {
        SPoint dragOffset = event.GetAt() - m_dragPoint;
        Panel::SetRenderPoint(m_preDragLocation + dragOffset);
        return true;
      }
      else
      {
        return PostDragHandle(event);
      }
    }

    virtual void OnMouseExit()
    {
      m_dragging = false;
      PostDragOnMouseExit();
    }

    virtual bool PostDragHandle(MouseButtonEvent& event)
    {
      return false;
    }

    virtual bool PostDragHandle(MouseMotionEvent& event)
    {
      return false;
    }

    virtual void PostDragOnMouseExit()
    { }
  };
} /* namespace MFM */
#endif /*PANEL_H*/
