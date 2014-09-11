/*                                              -*- mode:C++ -*-
  NeighborSelectPanel.h Panel for selecting neighbors in a Manhattan Neighborhood
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
  \file NeighborSelectPanel.h Panel for selecting neighbors in a Manhattan Neighborhood
  \author Trent R. Small
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef NEIGHBOR_SELECT_PANEL_H
#define NEIGHBOR_SELECT_PANEL_H

#include "Panel.h"

namespace MFM
{
  template <u32 R>
  class NeighborSelectPanel : public Panel
  {
   private:
    enum
    {
      CELL_SIZE = 16,
      BORDER_SIZE = 4
    };

    const char* m_text;

    static const u32 ENABLED_COLOR = 0xff20a020;

    BitVector<128> m_bitField;

    MDist<R> GetNeighborhood()
    {
      return MDist<R>::get();
    }

    void FlipBit(u32 bitNum)
    {
      if(IsBitOn(bitNum))
      {
        ClearBit(bitNum);
      }
      else
      {
        SetBit(bitNum);
      }
    }

    void SetBit(u32 bitNum)
    {
      m_bitField.WriteBit(bitNum, true);
    }

    void ClearBit(u32 bitNum)
    {
      m_bitField.WriteBit(bitNum, false);
    }

    bool IsBitOn(u32 bitNum)
    {
      return m_bitField.ReadBit(bitNum);
    }

   public:
    NeighborSelectPanel() :
      Panel()
    {
      Panel::SetDesiredSize(300, (2 * BORDER_SIZE) + (R * 2 + 1) * CELL_SIZE);
    }

    void SetText(const char* text)
    {
      m_text = text;
    }

    void PaintButtons(Drawing& d)
    {
      s32 offset = R * CELL_SIZE + BORDER_SIZE;
      SPoint renderPt;
      MDist<R> n = GetNeighborhood();

      for(u32 i = n.GetFirstIndex(0); i <= n.GetLastIndex(R); i++)
      {
        renderPt.Set(offset + (CELL_SIZE - 1) * n.GetPoint(i).GetX(),
                     offset + (CELL_SIZE - 1) * n.GetPoint(i).GetY());

        d.SetForeground(Drawing::BLACK);
        d.FillRect(renderPt.GetX(), renderPt.GetY(), CELL_SIZE, CELL_SIZE);

        if(IsBitOn(i - n.GetFirstIndex(0)))
        {
          d.SetForeground(ENABLED_COLOR);
        }
        else
        {
          d.SetForeground(Panel::GetForeground());
        }
        d.FillRect(renderPt.GetX() + 1, renderPt.GetY() + 1, CELL_SIZE - 2, CELL_SIZE - 2);
      }
    }

    virtual void PaintComponent(Drawing& d)
    {
      d.SetForeground(Panel::GetForeground());
      d.FillRect(0, 0, Panel::GetDimensions().GetX(), Panel::GetDimensions().GetY());

      PaintButtons(d);
    }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.type == SDL_MOUSEBUTTONDOWN &&
         event.m_event.button.button == SDL_BUTTON_LEFT)
      {
        s32 offset = R * CELL_SIZE + BORDER_SIZE;
        MDist<R> n = GetNeighborhood();
        Rect buttonRect;
        SPoint clickPt(event.GetAt().GetX() - Panel::GetRenderPoint().GetX(),
                       event.GetAt().GetY() - Panel::GetRenderPoint().GetY());
        buttonRect.SetSize(UPoint(CELL_SIZE, CELL_SIZE));

        for(u32 i = n.GetFirstIndex(0); i <= n.GetLastIndex(R); i++)
        {
          buttonRect.SetPosition(SPoint(offset + (CELL_SIZE - 1) * n.GetPoint(i).GetX(),
                                        offset + (CELL_SIZE - 1) * n.GetPoint(i).GetY()));

          if(buttonRect.Contains(event.GetAt()))
          {
            FlipBit(i - n.GetFirstIndex(0));
            return true;
          }
        }
      }
      return false;
    }
  };
}


#endif /* NEIGHBOR_SELECT_PANEL_H */
