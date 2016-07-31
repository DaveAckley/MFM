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
  template <class EC>
  class Slider : public ParameterController<EC>
  {
   private:
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    bool m_hitIn; // If button down was ours and we're still tracking motion

   public:

    enum
    {
      SLIDER_WIDTH = 150,
      SLIDER_HEIGHT = 50,

      SLIDER_WIDTH_BIG = 300,
      SLIDER_HEIGHT_BIG = 64,

      SLIDER_HALF_HEIGHT = (SLIDER_HEIGHT / 2),
      SLIDER_HALF_HEIGHT_BIG = (SLIDER_HEIGHT_BIG / 2),

      SLIDER_Y = 16,
      SLIDER_X_INDENTS = 12,
      SLIDER_TIC_HEIGHT = 4,
      SLIDER_TEXT_Y = SLIDER_Y + SLIDER_TIC_HEIGHT,

      SLIDER_ICON_WIDTH = 17, // ?? ICON SIZE?

      _UNUSED_COMMA_EATER_REMAINS_AT_END_
    };

    Slider() :
      m_hitIn(false)
    {
      Init();
    }

    inline u32 GetSliderWidth() const
    {
      return ParameterController<EC>::m_bigText ? SLIDER_WIDTH_BIG : SLIDER_WIDTH;
    }

    inline u32 GetSliderHeight() const
    {
      return ParameterController<EC>::m_bigText ? SLIDER_HEIGHT_BIG : SLIDER_HEIGHT;
    }

    inline u32 GetSliderHalfHeight() const
    {
      return ParameterController<EC>::m_bigText ? SLIDER_HALF_HEIGHT_BIG : SLIDER_HALF_HEIGHT;
    }

    inline FontAsset GetRenderFont() const
    {
      return
        ParameterController<EC>::m_bigText ? FONT_ASSET_BUTTON_BIG : FONT_ASSET_BUTTON_MEDIUM;
    }

    void Init()
    {
      Panel::SetDimensions(GetSliderWidth() * 2, GetSliderHeight());
      Panel::SetDesiredSize(10000, GetSliderHeight());
      Panel::SetFont(FONT_ASSET_BUTTON);
    }

    // We standardize on s32 for all values.  We blithely assume that,
    // for a slider, the range of u32 will not be large enough for
    // that to be a problem.
    s32 GetValue() const
    {
      MFM_API_ASSERT_NONNULL(this->m_parameter);
      MFM_API_ASSERT_NONNULL(this->m_patom);
      //      return this->m_parameter->GetBitsAsS32(*this->m_patom);
      return (s32) this->m_parameter->GetValueOutOfType(*this->m_patom);
    }

    s32 GetDefaultValue() const
    {
      MFM_API_ASSERT_NONNULL(this->m_parameter);
      MFM_API_ASSERT_NONNULL(this->m_patom);
      return this->m_parameter->GetDefault();
    }

    void SetValue(s32 val)
    {
      MFM_API_ASSERT_NONNULL(this->m_parameter);
      MFM_API_ASSERT_NONNULL(this->m_patom);
      //XXX      return this->m_parameter->SetBitsAsS32(*this->m_patom, val);
      this->m_parameter->SetValueIntoType(*this->m_patom, (u32) val);
    }

#if 0 // Tue Jul  7 00:25:28 2015 Not used?
    void SetSliderTarget(Parameter<EC>* parameter, Atom<EC> * patom)
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
#endif

    virtual void PaintBorder(Drawing& d)
    { /* No border*/ }

    virtual bool Handle(MouseButtonEvent& event)
    {
      m_hitIn = false; // Any button event except down in us means not us

      if(event.m_event.button.type == SDL_MOUSEBUTTONDOWN)
      {
        if(event.m_event.button.button == SDL_BUTTON_LEFT)
        {
          SPoint localpos = event.GetAt() - this->GetAbsoluteLocation();
          if(GetSliderHitArea().Contains(localpos))
          {
            m_hitIn = true;
            if(this->m_parameter)
            {
              this->SetValueToMappedX(localpos.GetX());
              return true;
            }
          }
          if (GetTextHitArea().Contains(localpos))
          {
            if(this->m_parameter)
            {
              this->SetValue(this->GetDefaultValue());
              return true;
            }
          }
        }
      }
      return false;
    }

    virtual bool Handle(MouseMotionEvent& event)
    {
      if(m_hitIn && event.m_event.button.button == SDL_BUTTON_LEFT)
      {
        SPoint localpos = event.GetAt() - this->GetAbsoluteLocation();
        if(GetSliderHitArea().Contains(localpos))
        {
          if(this->m_parameter)
          {
            this->SetValueToMappedX(localpos.GetX());
          }
        }
        // As long as the button's down, we're taking this event even
        // if it's outside the hit area, to hold on to the focus in
        // case it comes back into the hit area.
        return true;
      }
      return false;
    }

    virtual void OnMouseExit()
    {
      m_hitIn = false;
    }

    virtual void PaintComponent(Drawing & d)
    {
      if (!this->m_parameter)
      {
        return;
      }

      d.SetForeground(Drawing::BLACK);

      FontAsset font = GetRenderFont();
      d.SetFont(font);
      const u32 TEXT_HEIGHT = AssetManager::GetFontLineSkip(font);

      d.DrawHLine(SLIDER_Y, SLIDER_X_INDENTS, GetSliderWidth() - SLIDER_X_INDENTS);

      d.DrawVLine(SLIDER_X_INDENTS, SLIDER_Y, SLIDER_TEXT_Y);
      d.DrawVLine(GetSliderWidth() - SLIDER_X_INDENTS, SLIDER_Y, SLIDER_TEXT_Y);

      OString16 numBuffer;
      numBuffer.Printf("%d", this->m_parameter->GetMin());
      UPoint tsize;
      tsize = AssetManager::GetFontTextSize(font, numBuffer.GetZString());
      d.BlitText(numBuffer.GetZString(),
                 SPoint(SLIDER_X_INDENTS - tsize.GetX() / 2, SLIDER_TEXT_Y),
                 UPoint(tsize.GetX(), TEXT_HEIGHT));

      numBuffer.Reset();
      numBuffer.Printf("%d", this->m_parameter->GetMax());
      tsize = AssetManager::GetFontTextSize(font, numBuffer.GetZString());

      d.BlitText(numBuffer.GetZString(),
                 SPoint(GetSliderWidth() - tsize.GetX() / 2, SLIDER_TEXT_Y),
                 UPoint(tsize.GetX(), TEXT_HEIGHT));

      numBuffer.Reset();
      this->m_parameter->PrintValue(numBuffer, *this->m_patom);
      tsize = AssetManager::GetFontTextSize(font, numBuffer.GetZString());

      d.SetBackground(Drawing::GREY70);
      d.SetForeground(Drawing::WHITE);
      d.BlitBackedText(numBuffer.GetZString(),
                       SPoint(GetSliderWidth() / 2 - tsize.GetX() / 2, SLIDER_Y + 1),
                       UPoint(tsize.GetX(), TEXT_HEIGHT));

      Rect sliderRect = GetSliderRect();
      IconAsset ia;
      ia.SetIconSlot(ZSLOT_SLIDER_HANDLE);
      ia.SetEnabled(m_hitIn);
      d.BlitIconAsset(ia, sliderRect.GetHeight(), sliderRect.GetPosition());

      if (!IsAtDefault())
        d.SetForeground(Drawing::YELLOW);
      tsize = AssetManager::GetFontTextSize(font, this->m_parameter->GetName());
      d.BlitBackedText(this->m_parameter->GetName(),
                       SPoint(GetSliderWidth(), 0),
                       UPoint(tsize.GetX(), TEXT_HEIGHT));
    }

  private:

    bool IsAtDefault() const
    {
      return this->GetValue() == this->GetDefaultValue();
    }

    void SetValueToMappedX(u32 xhit)
    {
      SetValue(MapXToParmVal(xhit));
    }

    s32 MapXToParmVal(u32 xhit)
    {
      double frac = 1.0 * ((s32) xhit - SLIDER_X_INDENTS) / (GetSliderWidth() - 2 * SLIDER_X_INDENTS);
      frac = CLAMP<double>(0.0, 1.0, frac);
      s32 min = this->m_parameter->GetMin();
      s32 max = this->m_parameter->GetMax();
      return (s32) ((frac * (max - min)) + min);
    }

    inline Rect GetSliderHitArea() const
    {
      // Add a buffer around the slider, for less fussy GUI, and (in
      // width) to ensure we can reach min/max despite granularity
      const s32 HIT_WIDTH_BUFFER = SLIDER_X_INDENTS / 2;
      const s32 HIT_HEIGHT_BUFFER = SLIDER_Y / 2;
      return Rect(SLIDER_X_INDENTS - HIT_WIDTH_BUFFER,
                  0 - HIT_HEIGHT_BUFFER,
                  GetSliderWidth() - 2 * SLIDER_X_INDENTS + 2 * HIT_WIDTH_BUFFER,
                  SLIDER_Y + 2 * HIT_HEIGHT_BUFFER);
    }

    inline Rect GetTextHitArea() const
    {
      FontAsset font = GetRenderFont();
      const u32 TEXT_HEIGHT = AssetManager::GetFontLineSkip(font);

      UPoint tsize = AssetManager::GetFontTextSize(font, this->m_parameter->GetName());
      return Rect(GetSliderWidth(), 0, tsize.GetX(), TEXT_HEIGHT);
    }

    inline Rect GetSliderRect() const
    {
      s32 xpos = 0;
      if (this->m_parameter)
      {
        s32 val = GetValue();
        xpos =
          this->m_parameter->MapValue((GetSliderWidth() - 2 * SLIDER_X_INDENTS), val)
          + SLIDER_X_INDENTS
          - SLIDER_ICON_WIDTH / 2;
      }
      return Rect(xpos >= 0 ? xpos : 0, 0, SLIDER_ICON_WIDTH, GetSliderHalfHeight());
    }
  };
}

#endif /* SLIDER_H */
