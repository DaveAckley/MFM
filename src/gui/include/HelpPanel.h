 /*                                              -*- mode:C++ -*-
  HelpPanel.h Panel for displaying controls and other help information
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
  \file HelpPanel.h Panel for displaying controls and other help information
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef HELPPANEL_H
#define HELPPANEL_H

#include "AssetManager.h"
#include "MovablePanel.h"

namespace MFM
{
  /**
   * A Panel which displays helpful information about the driver it
   * resides in.
   */
  class HelpPanel : public MovablePanel
  {
  private:

    /**
     * The messages which are rendered upon rendering of this
     * HelpPanel .
     */
    static const char* m_helpMessages[];

  public:
    /**
     * Constructs a new HelpPanel that is ready to be used.
     */
    HelpPanel()
    {
      Panel::SetBackground(Drawing::DARK_PURPLE);
      Panel::SetAnchor(ANCHOR_WEST);
      Panel::SetAnchor(ANCHOR_SOUTH);
    }

    virtual void PaintBorder(Drawing& d)
    { /* No border */ }

    virtual void PaintComponent(Drawing& d)
    {
      TTF_Font* bigFont = AssetManager::Get(FONT_ASSET_HELPPANEL_BIG);
      TTF_Font* smFont  = AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL);

      d.SetForeground(Panel::GetBackground());
      d.FillRect(0, 0, Panel::GetWidth(), Panel::GetHeight());

      d.SetForeground(Drawing::WHITE);
      d.SetFont(bigFont);

      d.BlitText("Help", UPoint(5, 5), MakeUnsigned(Panel::GetTextSize(bigFont, "Help")));

      d.SetFont(smFont);
      UPoint maxCorner(0,0);

      for(u32 i = 0; m_helpMessages[i]; i++)
      {
        UPoint tsize = MakeUnsigned(Panel::GetTextSize(smFont, m_helpMessages[i]));
        UPoint pos = UPoint(10, i * 18 + 35);
        d.BlitText(m_helpMessages[i], pos, tsize);

        maxCorner = max(maxCorner, pos + tsize);
      }
      maxCorner += UPoint(10,10);

      if (maxCorner != this->GetDesiredSize())
      {
        this->SetDimensions(maxCorner.GetX(), maxCorner.GetY());
        this->SetDesiredSize(maxCorner.GetX(), maxCorner.GetY());
        if (Panel::m_parent != 0)
        {
          Panel::HandleResize(Panel::m_parent->GetDimensions());
        }
      }
    }
  };
}

#endif /* HELPPANEL_H */
