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
#include "Parameter.h"
#include "ParameterController.h"

namespace MFM
{
  template <class CC>
  class Slider : public ParameterController<CC>
  {
  private:
    typedef typename CC::ATOM_TYPE T;

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
      m_dragging(false)
    {
      Init();
    }

    void Init()
    {
      Panel::SetDimensions(SLIDER_WIDTH * 2, SLIDER_HEIGHT);
      Panel::SetDesiredSize(10000, SLIDER_HEIGHT);
    }

    // We standardize on s32 for all values.  We blithely assume that,
    // for a slider, the range of u32 will not be large enough for
    // that to be a problem.
    s32 GetValue()
    {
      MFM_API_ASSERT_NONNULL(this->m_parameter);
      MFM_API_ASSERT_NONNULL(this->m_patom);
      return this->m_parameter->GetBitsAsS32(*this->m_patom);
    }

    void SetValue(s32 val)
    {
      MFM_API_ASSERT_NONNULL(this->m_parameter);
      MFM_API_ASSERT_NONNULL(this->m_patom);
      return this->m_parameter->SetBitsAsS32(*this->m_patom, val);
    }

    void SetSliderTarget(Parameter<CC>* parameter, Atom<CC> * patom)
    {
      if (!parameter && !patom)
      {
        this->m_parameter = NULL;
        this->m_patom = NULL;
        return;
      }

      if (!parameter || !patom)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      this->m_parameter = parameter;
      this->m_patom = patom;
    }

    virtual void PaintBorder(Drawing& d)
    { /* No border*/ }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.button.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.m_event.button.button == SDL_BUTTON_LEFT)
        {
          if(GetSliderRect().Contains(event.GetAt() - this->GetAbsoluteLocation()))
          {
            if(this->m_parameter)
            {
              m_dragging = true;
              m_dragStartX = event.GetAt().GetX();
              m_preDragVal = GetValue();
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
      if(m_dragging && this->m_parameter)
      {
        s32 delta = event.m_event.motion.x - m_dragStartX;
        s32 valDelta = ((delta * this->m_parameter->GetMax()) - 16) / (SLIDER_WIDTH - 16);
        if (this->m_parameter)
        {
          SetValue(m_preDragVal + valDelta);
        }
      }
      return m_dragging;
    }

    virtual void OnMouseExit()
    {
      m_dragging = false;
    }

    virtual void PaintComponent(Drawing & d)
    {
      if (!this->m_parameter)
      {
        return;
      }

      d.SetForeground(Drawing::BLACK);

      d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));

      d.DrawHLine(16, 7, SLIDER_WIDTH - 7);

      d.DrawVLine(7, 16, 20);
      d.DrawVLine(SLIDER_WIDTH - 7, 16, 20);

      CharBufferByteSink<16> numBuffer;
      numBuffer.Printf("%d", this->m_parameter->GetMin());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(3, 16),
                 UPoint(48, 16));

      numBuffer.Reset();
      numBuffer.Printf("%d", this->m_parameter->GetMax());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(SLIDER_WIDTH - 32, SLIDER_HALF_HEIGHT),
                 UPoint(48, SLIDER_HALF_HEIGHT));

      numBuffer.Reset();
      this->m_parameter->PrintValue(numBuffer, *this->m_patom);

      d.SetBackground(Drawing::GREY70);
      d.SetForeground(Drawing::WHITE);
      d.BlitBackedText(numBuffer.GetZString(),
                       UPoint(SLIDER_WIDTH / 2 - 16, SLIDER_HALF_HEIGHT),
                       UPoint(48, SLIDER_HALF_HEIGHT));

      Rect sliderRect = GetSliderRect();
      d.BlitAsset(ASSET_SLIDER_HANDLE,
                  MakeUnsigned(sliderRect.GetPosition()),
                  sliderRect.GetSize());

      d.BlitBackedText(this->m_parameter->GetName(),
                       UPoint(SLIDER_WIDTH, 7),
                       UPoint(SLIDER_WIDTH, SLIDER_HALF_HEIGHT));
    }

  private:

    inline Rect GetSliderRect()
    {
      s32 xpos = 0;
      if (this->m_parameter)
      {
        s32 val = GetValue();
        xpos = this->m_parameter->MapValue((SLIDER_WIDTH - 7), val) - 7;
      }
      return Rect(xpos > 0 ? xpos : 0, 0, 17, SLIDER_HALF_HEIGHT);
    }
  };
}

#endif /* SLIDER_H */
