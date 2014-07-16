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
#include "Panel.h"

#define HELP_MESSAGE_COUNT 25

namespace MFM
{
  /**
   * A Panel which displays helpful information about the driver it
   * resides in.
   */
  class HelpPanel : public Panel
  {
  private:

    const char* m_helpMessages[HELP_MESSAGE_COUNT];

    void RegisterMessages()
    {
      const char** messages = m_helpMessages;
      *(messages++) = "";
      *(messages++) = "Keyboard:";
      *(messages++) = " [t] Show toolbox";
      *(messages++) = " [g] Show grid";
      *(messages++) = " [m] Toggle memory view";
      *(messages++) = " [k] Render Atom heatmap";
      *(messages++) = " [h] Show this help window";
      *(messages++) = " [l] Show log";
      *(messages++) = " [p] Render caches";
      *(messages++) = " [esc] Deselect Tile";
      *(messages++) = " [r] Toggle recording";
      *(messages++) = " [arrow keys] Move tiles";
      *(messages++) = " [,] Decrease AEPS per frame";
      *(messages++) = " [.] Increase AEPS per frame";
      *(messages++) = " [i] Show statistics and settings";
      *(messages++) = "    [a] Show AER statistics";
      *(messages++) = " [CTRL+q] Quit";
      *(messages++) = "";
      *(messages++) = "Mouse:";
      *(messages++) = " [Ctrl+Drag] Move tiles";
      *(messages++) = " [Scroll] Zoom";
      *(messages++) = "";
      *(messages++) = "Painting:";
      *(messages++) = " [Left Click] Select / paint element 1";
      *(messages++) = " [Right Click] Select / paint element 2";
    }

  public:

    HelpPanel()
    {
      RegisterMessages();
      Panel::SetBackground(Drawing::DARK_PURPLE);
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

      d.BlitText("Help", UPoint(0, 0), MakeUnsigned(Panel::GetTextSize(bigFont, "Help")));

      d.SetFont(smFont);
      for(u32 i = 0; i < HELP_MESSAGE_COUNT; i++)
      {
	d.BlitText(m_helpMessages[i],
		   UPoint(0, i * 16 + 28),
		   MakeUnsigned(Panel::GetTextSize(smFont, m_helpMessages[i])));
      }
    }
  };
}

#endif /* HELPPANEL_H */
