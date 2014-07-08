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
#include "Drawing.h"
#include "Panel.h"
#include "Util.h"

#define SLIDER_WIDTH 128

namespace MFM
{
  class Slider : public Panel
  {
  private:
    u32 m_minValue;
    u32 m_maxValue;
    u32 m_value;

    bool m_dragging;

    const char* m_text;

  public:

    Slider() :
      m_minValue(0),
      m_maxValue(100),
      m_value(50),
      m_dragging(false),
      m_text(NULL)
    {
      Init();
    }

    Slider(u32 maxValue, u32 minValue) :
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_value((maxValue - minValue) / 2),
      m_dragging(false),
      m_text(NULL)
    {
      Init();
    }

    Slider(u32 maxValue, u32 minValue, u32 value) :
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_value(value),
      m_dragging(false),
      m_text(NULL)
    {
      Init();
    }

    void Init()
    {
      Panel::SetDimensions(SLIDER_WIDTH, 32);
    }

    void SetMinValue(u32 value)
    {
      m_minValue = value;
    }

    u32 GetMinValue() const
    {
      return m_minValue;
    }

    void SetMaxValue(u32 value)
    {
      m_maxValue = value;
    }

    u32 GetMaxValue() const
    {
      return m_maxValue;
    }

    void SetValue(u32 value)
    {
      m_value = CLAMP(m_minValue, m_maxValue, value);
    }

    u32 GetValue() const
    {
      return m_value;
    }

    virtual void PaintBorder(Drawing& d)
    {/* No border*/}

    virtual void PaintComponent(Drawing & d)
    {
      SetValue(m_value - 1);

      d.SetForeground(Drawing::BLACK);

      d.DrawHLine(16, 7, SLIDER_WIDTH - 7);

      d.DrawVLine(7, 16, 20);
      d.DrawVLine(SLIDER_WIDTH - 7, 16, 20);

      /* TODO Render numbers at hi and low ends */

      d.BlitAsset(ASSET_SLIDER_HANDLE,
		  UPoint((m_value * (SLIDER_WIDTH - 16)) / m_maxValue, 0),
	          UPoint(17, 16));
    }
  };
}

#endif /* SLIDER_H */
