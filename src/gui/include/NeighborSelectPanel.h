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
#include "Parameter.h"
#include "ParameterController.h"

namespace MFM
{
  template <class CC, u32 R>
  class NeighborSelectPanel : public ParameterController<CC>
  {
   private:
    typedef ParameterController<CC> Super;

    enum
    {
      SITES = EVENT_WINDOW_SITES(R)
    };

   public:
    typedef enum
    {
      NEIGHBOR_SELECT_MANY = 0,
      NEIGHBOR_SELECT_ONE
    }NeighborhoodSelectMode;

   private:
    enum
    {
      CELL_SIZE = 16,
      BORDER_SIZE = 4,

      CELL_SIZE_BIG = 24
    };

    inline u32 GetCellSize() const
    {
      return Super::m_bigText ?
      CELL_SIZE_BIG :
      CELL_SIZE;
    }

    inline FontAsset GetFontAsset() const
    {
      return Super::m_bigText ?
      FONT_ASSET_HELPPANEL_BIG :
      FONT_ASSET_HELPPANEL_SMALL;
    }

    NeighborhoodSelectMode m_selectMode;

    SPoint m_selectedOne;

    BitVector<SITES> m_bitField;

    const char* m_text;

    static const u32 ENABLED_COLOR = 0xff20a020;

    const MDist<R> & GetNeighborhood() const
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
      if(ParameterController<CC>::GetParameter())
      {
        ElementParameterNeighborhood<CC,SITES>* np =
          dynamic_cast<ElementParameterNeighborhood<CC,SITES>*>
            (ParameterController<CC>::GetParameter());

        np->SetBit(bitNum);
      }
    }

    void ClearBit(u32 bitNum)
    {
      m_bitField.WriteBit(bitNum, false);
      if(ParameterController<CC>::GetParameter())
      {
        ElementParameterNeighborhood<CC,SITES>* np =
          dynamic_cast<ElementParameterNeighborhood<CC,SITES>*>
            (ParameterController<CC>::GetParameter());

        np->ClearBit(bitNum);
      }
    }

    bool IsBitOn(u32 bitNum)
    {
      return m_bitField.ReadBit(bitNum);
    }

    void PaintButtons(Drawing& d)
    {
      s32 offset = R * GetCellSize() + BORDER_SIZE;
      SPoint renderPt;
      const MDist<R> & n = GetNeighborhood();

      for(u32 i = n.GetFirstIndex(0); i <= n.GetLastIndex(R); i++)
      {
        renderPt.Set(offset + (GetCellSize() - 1) * n.GetPoint(i).GetX(),
                     offset + (GetCellSize() - 1) * n.GetPoint(i).GetY());

        d.SetForeground(Drawing::BLACK);
        d.FillRect(renderPt.GetX(), renderPt.GetY(), GetCellSize(), GetCellSize());

        switch(m_selectMode)
        {
        case NEIGHBOR_SELECT_MANY:
          d.SetForeground(IsBitOn(i - n.GetFirstIndex(0)) ?
                          ENABLED_COLOR : Panel::GetForeground());
          break;
        case NEIGHBOR_SELECT_ONE:
          d.SetForeground(n.GetPoint(i).Equals(m_selectedOne) ?
                          ENABLED_COLOR : Panel::GetForeground());
        }
        d.FillRect(renderPt.GetX() + 1, renderPt.GetY() + 1, GetCellSize() - 2, GetCellSize() - 2);
      }
    }

    void PaintText(Drawing& d)
    {
      UPoint textLoc(R * (GetCellSize() + 1) * 2 + GetCellSize() + BORDER_SIZE,
                     R * (GetCellSize() - 1) + BORDER_SIZE);

      d.SetForeground(Drawing::WHITE);
      d.SetBackground(Drawing::GREY40);
      d.SetFont(GetFontAsset());
      if(m_text)
      {
        d.BlitBackedText(m_text, textLoc, MakeUnsigned(d.GetTextSize(m_text)));
      }
    }

   public:
    NeighborSelectPanel() :
      ParameterController<CC>(),
      m_selectMode(NEIGHBOR_SELECT_MANY),
      m_selectedOne(0, 0)
    {
      Init();
    }

    virtual void Init()
    {
      Panel::SetDesiredSize(500, (2 * BORDER_SIZE) + (R * 2 + 1) * GetCellSize());
      Panel::SetForeground(Drawing::GREY80);
    }

    void SetParameter(ElementParameter<CC>* pb)
    {
      Super::SetParameter(pb);
      ElementParameterNeighborhood<CC,SITES>* np =
        dynamic_cast<ElementParameterNeighborhood<CC,SITES>*>
          (ParameterController<CC>::GetParameter());

      if (!np)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      u64 ngb = np->GetValue();
      m_bitField.WriteLong(0, SITES, ngb);
      LOG.Debug("Neighborhood initted to %@", np);
    }

    void SetSelectMode(NeighborhoodSelectMode mode)
    {
      m_selectMode = mode;
      m_bitField.Clear();
      m_selectedOne(0, 0);
    }

    void SetText(const char* text)
    {
      m_text = text;
    }

    virtual void PaintComponent(Drawing& d)
    {
      d.SetForeground(Panel::GetForeground());
      d.FillRect(0, 0, Panel::GetDimensions().GetX(), Panel::GetDimensions().GetY());

      PaintButtons(d);
      PaintText(d);
    }

    virtual bool Handle(MouseButtonEvent& event)
    {
      if(event.m_event.type == SDL_MOUSEBUTTONDOWN &&
         event.m_event.button.button == SDL_BUTTON_LEFT)
      {
        s32 offset = R * GetCellSize() + BORDER_SIZE;
        const MDist<R> & n = GetNeighborhood();
        Rect buttonRect;
        SPoint clickPt(event.GetAt().GetX() - Panel::GetRenderPoint().GetX(),
                       event.GetAt().GetY() - Panel::GetRenderPoint().GetY());
        buttonRect.SetSize(UPoint(GetCellSize(), GetCellSize()));

        for(u32 i = n.GetFirstIndex(0); i <= n.GetLastIndex(R); i++)
        {
          buttonRect.SetPosition(SPoint(offset + (GetCellSize() - 1) * n.GetPoint(i).GetX(),
                                        offset + (GetCellSize() - 1) * n.GetPoint(i).GetY()));

          if(buttonRect.Contains(clickPt))
          {
            switch(m_selectMode)
            {
            case NEIGHBOR_SELECT_MANY:
              FlipBit(i - n.GetFirstIndex(0));
              break;
            case NEIGHBOR_SELECT_ONE:
              m_selectedOne.Set(n.GetPoint(i).GetX(), n.GetPoint(i).GetY());
              break;
            }
            return true;
          }
        }
      }
      return false;
    }
  };
}


#endif /* NEIGHBOR_SELECT_PANEL_H */
