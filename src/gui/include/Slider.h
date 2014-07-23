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
#include "Panel.h"
#include "Util.h"

#define SLIDER_WIDTH 150

namespace MFM
{
  class Slider : public Panel
  {
  private:
    s32 m_minValue;
    s32 m_maxValue;

    s32 m_snapResolution;

    /**
     * If not \c NULL, will set this value alongside the value inside
     * the Slider. This is useful for binding some outside value to
     * this Slider as long as it does not change outside the Slider.
     */
    s32* m_externalValue;

    bool m_dragging;
    s32 m_dragStartX;
    s32 m_preDragVal;

    const char* m_text;

  public:

    Slider() :
      m_minValue(0),
      m_maxValue(100),
      m_snapResolution(1),
      m_externalValue(NULL),
      m_dragging(false),
      m_text(NULL)
    {
      Init();
    }

    Slider(s32 maxValue, s32 minValue) :
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_externalValue(NULL),
      m_dragging(false),
      m_text(NULL)
    {
      Init();
    }

    void Init()
    {
      Panel::SetDimensions(SLIDER_WIDTH * 2, 32);
    }

    void SetExternalValue(s32* externalValue)
    {
      m_externalValue = externalValue;
    }

    void SetText(const char* text)
    {
      m_text = text;
    }

    void SetMinValue(s32 value)
    {
      m_minValue = value;
    }

    s32 GetMinValue() const
    {
      return m_minValue;
    }

    void SetMaxValue(s32 value)
    {
      m_maxValue = value;
    }

    s32 GetMaxValue() const
    {
      return m_maxValue;
    }

    void SetValue(s32 value)
    {
      if(m_externalValue)
      {
        s32 newVal = CLAMP(m_minValue, m_maxValue, value);
        newVal -= (newVal % m_snapResolution);
        newVal = CLAMP(m_minValue, m_maxValue, newVal);
        *m_externalValue = newVal;
      }
    }

    s32 GetValue() const
    {
      return *m_externalValue;
    }

    void SetSnapResolution(s32 snapResolution)
    {
      m_snapResolution = snapResolution;
    }

    s32 GetSnapResolution() const
    {
      return m_snapResolution;
    }

    virtual void PaintBorder(Drawing& d)
    {/* No border*/}

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.button.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.m_event.button.button == SDL_BUTTON_LEFT)
        {
          if(GetSliderRect().Contains(event.GetAt() - GetAbsoluteLocation()))
          {
            if(m_externalValue)
            {
              m_dragging = true;
              m_dragStartX = event.GetAt().GetX();
              m_preDragVal = *m_externalValue;
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
      if(m_dragging)
      {
        s32 delta = event.m_event.motion.x - m_dragStartX;
        s32 valDelta = (delta * m_maxValue) / SLIDER_WIDTH;

        SetValue(m_preDragVal + valDelta);
      }
      return m_dragging;
    }

    virtual void OnMouseExit()
    {
      m_dragging = false;
    }

    virtual void PaintComponent(Drawing & d)
    {
      d.SetForeground(Drawing::BLACK);

      d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));

      d.DrawHLine(16, 7, SLIDER_WIDTH - 7);

      d.DrawVLine(7, 16, 20);
      d.DrawVLine(SLIDER_WIDTH - 7, 16, 20);

      CharBufferByteSink<16> numBuffer;
      numBuffer.Printf("%d", m_minValue);
      d.BlitText(numBuffer.GetZString(),
                 UPoint(3, 16),
                 UPoint(48, 16));

      numBuffer.Reset();
      numBuffer.Printf("%d", m_maxValue);
      d.BlitText(numBuffer.GetZString(),
                 UPoint(SLIDER_WIDTH - 32, 16),
                 UPoint(48, 16));

      numBuffer.Reset();

      if(m_externalValue)
      {
        numBuffer.Printf("%d", *m_externalValue);
      }
      else
      {
        numBuffer.Printf("?");
      }

      d.SetBackground(Drawing::BLACK);
      d.SetForeground(Drawing::WHITE);
      d.BlitBackedText(numBuffer.GetZString(),
                       UPoint(SLIDER_WIDTH / 2 - 16, 16),
                       UPoint(48, 16));

      Rect sliderRect = GetSliderRect();
      d.BlitAsset(ASSET_SLIDER_HANDLE,
                  MakeUnsigned(sliderRect.GetPosition()),
                  sliderRect.GetSize());

      if(m_text)
      {
        d.BlitBackedText(m_text,
                         UPoint(SLIDER_WIDTH, 7),
                         UPoint(SLIDER_WIDTH, 16));
      }
    }

  private:

    inline Rect GetSliderRect()
    {
      s32 xpos = (((*m_externalValue) * (SLIDER_WIDTH - 7)) / m_maxValue) - 7;
      return Rect(xpos > 0 ? xpos : 0,
                  0, 17, 16);
    }
  };
}

#endif /* SLIDER_H */
