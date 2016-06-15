/*                                              -*- mode:C++ -*-
  MovablePanel.h Dragging layer for panels which can be dragged
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
  \file MovablePanel.h Dragging layer for panels which can be dragged
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
    typedef Panel Super;
   private:
    SPoint m_dragPoint;
    SPoint m_preDragLocation;
    SPoint m_preDragSize;
    bool m_dragging;
    bool m_ctrlDragging;
    bool m_movable;
    bool m_resizable;

   public:
    MovablePanel(u32 width = 0, u32 height = 0)
      : Panel(width, height)
      , m_dragging(false)
      , m_ctrlDragging(false)
      , m_movable(true)
      , m_resizable(true)
    { }


    void SetMovable(bool canmove)
    {
      m_movable = canmove;
    }
    void SetResizable(bool canresize)
    {
      m_resizable = canresize;
    }
    bool IsMovable() const
    {
      return m_movable;
    }
    bool IsResizable() const
    {
      return m_resizable;
    }

    void BeginDrag(MouseButtonEvent& event, bool ctrl)
    {
      m_dragging = true;
      m_ctrlDragging = ctrl;
      m_dragPoint = event.GetAt();
      m_preDragLocation = Panel::GetRenderPoint();
      m_preDragSize = MakeSigned(Panel::GetDimensions());
    }

    void EndDrag()
    {
      m_dragging = false;
      m_ctrlDragging = false;
    }

    virtual void SaveDetails(ByteSink& sink) const
    {
      Super::SaveDetails(sink);
    }

    virtual bool LoadDetails(const char * key, LineCountingByteSource& source)
    {
      return Super::LoadDetails(key, source);
    }

    virtual bool Handle(KeyboardEvent& event)
    {
      return Super::Handle(event);
    }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.type == SDL_MOUSEBUTTONDOWN)
      {
        if(IsMovable() && (event.m_keyboardModifiers & KMOD_CTRL))
        {
          BeginDrag(event, event.m_keyboardModifiers & KMOD_CTRL);
          return true;
        }
        if(IsResizable() && (event.m_keyboardModifiers & KMOD_SHIFT))
        {
          BeginDrag(event, event.m_keyboardModifiers & KMOD_CTRL);
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
        if (m_ctrlDragging)
        {
          SPoint newRender = m_preDragLocation + dragOffset;
          Panel::SetRenderPoint(newRender);
          LOG.Debug("[%s] render point: (%d,%d)",
                    this->GetName(),
                    newRender.GetX(),
                    newRender.GetY());
        }
        else
        {
          const s32 MIN_WINDOW_EDGE = 10;
          SPoint newsize =
            max(m_preDragSize + dragOffset,
                SPoint(MIN_WINDOW_EDGE,MIN_WINDOW_EDGE));
          Panel::SetDimensions(newsize.GetX(), newsize.GetY());
          Panel::SetDesiredSize(newsize.GetX(), newsize.GetY());
          LOG.Debug("[%s] size: (%d,%d)",
                    this->GetName(),
                    newsize.GetX(),
                    newsize.GetY());
        }
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

    // By default, eat all other click and motions inside a movable panel
    virtual bool PostDragHandle(MouseButtonEvent& event)
    {
      return true;
    }

    virtual bool PostDragHandle(MouseMotionEvent& event)
    {
      return true;
    }

    virtual void PostDragOnMouseExit()
    { }
  };
} /* namespace MFM */
#endif /*PANEL_H*/
