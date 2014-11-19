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

    enum
    {
      SLIDER_WIDTH = 150,
      SLIDER_HEIGHT = 32,

      SLIDER_WIDTH_BIG = 300,
      SLIDER_HEIGHT_BIG = 64,

      SLIDER_HALF_HEIGHT = (SLIDER_HEIGHT / 2),
      SLIDER_HALF_HEIGHT_BIG = (SLIDER_HEIGHT_BIG / 2)
    };

    Slider() :
      m_dragging(false)
    {
      Init();
    }

    inline u32 GetSliderWidth() const
    {
      return ParameterController<CC>::m_bigText ? SLIDER_WIDTH_BIG : SLIDER_WIDTH;
    }

    inline u32 GetSliderHeight() const
    {
      return ParameterController<CC>::m_bigText ? SLIDER_HEIGHT_BIG : SLIDER_HEIGHT;
    }

    inline u32 GetSliderHalfHeight() const
    {
      return ParameterController<CC>::m_bigText ? SLIDER_HALF_HEIGHT_BIG : SLIDER_HALF_HEIGHT;
    }

    inline FontAsset GetRenderFont() const
    {
      return ParameterController<CC>::m_bigText ? FONT_ASSET_ELEMENT :
      FONT_ASSET_HELPPANEL_SMALL;
    }

    void Init()
    {
      Panel::SetDimensions(GetSliderWidth() * 2, GetSliderHeight());
      Panel::SetDesiredSize(10000, GetSliderHeight());
    }

    // We standardize on s32 for all values.  We blithely assume that,
    // for a slider, the range of u32 will not be large enough for
    // that to be a problem.
    s32 GetValue() const
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
        /* Might want the things underneath to let up too. */
        return false;
      }
      return false;
    }

    virtual bool Handle(MouseMotionEvent& event)
    {
      if(m_dragging && this->m_parameter)
      {
        s32 delta = event.m_event.motion.x - m_dragStartX;
        s32 valDelta = delta * ((double)this->m_parameter->GetMax() / GetSliderWidth());

        if(delta < 0)
        {
          if(valDelta >= 0)
          {
            valDelta = 0;
          }
        }
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

      d.SetFont(AssetManager::Get(GetRenderFont()));

      d.DrawHLine(16, 7, GetSliderWidth() - 7);

      d.DrawVLine(7, 16, 20);
      d.DrawVLine(GetSliderWidth() - 7, 16, 20);

      CharBufferByteSink<16> numBuffer;
      numBuffer.Printf("%d", this->m_parameter->GetMin());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(3, 16),
                 UPoint(48, 16));

      numBuffer.Reset();
      numBuffer.Printf("%d", this->m_parameter->GetMax());
      d.BlitText(numBuffer.GetZString(),
                 UPoint(GetSliderWidth() - 32, GetSliderHalfHeight()),
                 UPoint(48, GetSliderHalfHeight()));

      numBuffer.Reset();
      this->m_parameter->PrintValue(numBuffer, *this->m_patom);

      d.SetBackground(Drawing::GREY70);
      d.SetForeground(Drawing::WHITE);
      d.BlitBackedText(numBuffer.GetZString(),
                       UPoint(GetSliderWidth() / 2 - 16, GetSliderHalfHeight()),
                       UPoint(48, GetSliderHalfHeight()));

      Rect sliderRect = GetSliderRect();
      d.BlitAsset(ASSET_SLIDER_HANDLE,
                  MakeUnsigned(sliderRect.GetPosition()),
                  sliderRect.GetSize());

      d.BlitBackedText(this->m_parameter->GetName(),
                       UPoint(GetSliderWidth(), 7),
                       UPoint(GetSliderWidth(), GetSliderHalfHeight()));
    }

  private:

    inline Rect GetSliderRect() const
    {
      s32 xpos = 0;
      if (this->m_parameter)
      {
        s32 val = GetValue();
        xpos = this->m_parameter->MapValue((GetSliderWidth() - 7), val) - 7;
      }
      return Rect(xpos > 0 ? xpos : 0, 0, 17, GetSliderHalfHeight());
    }
  };
}

#endif /* SLIDER_H */
