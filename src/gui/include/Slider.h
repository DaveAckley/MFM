/*                                              -*- mode:C++ -*-
  Slider.h GUI Slider for tweaking numeric values
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
  \file Slider.h GUI Slider for tweaking numeric values
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef SLIDER_H
#define SLIDER_H

#include "AssetManager.h"
#include "CharBufferByteSink.h"
#include "Drawing.h"
#include "Util.h"
#include "Parameters.h"
#include "ParameterController.h"

namespace MFM
{
  class Slider : public ParameterController
  {
  private:
    Parameters::S32 * m_parm;

    bool m_dragging;
    s32 m_dragStartX;
    s32 m_preDragVal;

  public:

    enum {
      SLIDER_WIDTH = 150,
      SLIDER_HEIGHT = 32,
      SLIDER_HALF_HEIGHT = (SLIDER_HEIGHT / 2)
    };

    Slider() :
      m_parm(0),
      m_dragging(false)
    {
      Init();
    }

    void Init()
    {
      Panel::SetDimensions(SLIDER_WIDTH * 2, SLIDER_HEIGHT);
      Panel::SetDesiredSize(10000, SLIDER_HEIGHT);
    }

    void SetParameter(Parameters::S32* parameter)
    {
      m_parm = parameter;
      if (m_parm)
      {
        m_parm->SetValue(m_parm->GetValue());
      }
    }

    virtual void PaintBorder(Drawing& d)
    { /* No border*/ }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.button.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.m_event.button.button == SDL_BUTTON_LEFT)
        {
          if(GetSliderRect().Contains(event.GetAt() - GetAbsoluteLocation()))
          {
            if(m_parm)
            {
              m_dragging = true;
              m_dragStartX = event.GetAt().GetX();
              m_preDragVal = m_parm->GetValue();
              return true;
            }
          }
        }
      }
      else
      {
        m_dragging = false;
        return true;
      }
      return false;
    }

    virtual bool Handle(MouseMotionEvent& event)
    {
      if(m_dragging && m_parm)
      {
        s32 delta = event.m_event.motion.x - m_dragStartX;
        s32 valDelta = ((delta * m_parm->GetMax()) - 16) / (SLIDER_WIDTH - 16);

        m_parm->SetValue(m_preDragVal + valDelta);
      }
      return m_dragging;
    }

    virtual void OnMouseExit()
    {
      m_dragging = false;
    }

    virtual void PaintComponent(Drawing & d)
    {
      if (!m_parm)
      {
        return;
      }

      d.SetForeground(Drawing::BLACK);

      d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));

      d.DrawHLine(16, 7, SLIDER_WIDTH - 7);

      d.DrawVLine(7, 16, 20);
      d.DrawVLine(SLIDER_WIDTH - 7, 16, 20);

      CharBufferByteSink<16> numBuffer;
      numBuffer.Printf("%d", m_parm->GetMin());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(3, 16),
                 UPoint(48, 16));

      numBuffer.Reset();
      numBuffer.Printf("%d", m_parm->GetMax());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(SLIDER_WIDTH - 32, SLIDER_HALF_HEIGHT),
                 UPoint(48, SLIDER_HALF_HEIGHT));

      numBuffer.Reset();
      numBuffer.Printf("%d", m_parm->GetValue());

      d.SetBackground(Drawing::GREY70);
      d.SetForeground(Drawing::WHITE);
      d.BlitBackedText(numBuffer.GetZString(),
                       UPoint(SLIDER_WIDTH / 2 - 16, SLIDER_HALF_HEIGHT),
                       UPoint(48, SLIDER_HALF_HEIGHT));

      Rect sliderRect = GetSliderRect();
      d.BlitAsset(ASSET_SLIDER_HANDLE,
                  MakeUnsigned(sliderRect.GetPosition()),
                  sliderRect.GetSize());

      d.BlitBackedText(m_parm->GetName(),
                       UPoint(SLIDER_WIDTH, 7),
                       UPoint(SLIDER_WIDTH, SLIDER_HALF_HEIGHT));
    }

  private:

    inline Rect GetSliderRect()
    {
      s32 xpos = 0;
      if (m_parm)
      {
        xpos = m_parm->MapValue((SLIDER_WIDTH - 7), m_parm->GetValue()) - 7;
      }
      return Rect(xpos > 0 ? xpos : 0, 0, 17, SLIDER_HALF_HEIGHT);
    }
  };
}

#endif /* SLIDER_H */
