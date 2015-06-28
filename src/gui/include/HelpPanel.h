 /*                                              -*- mode:C++ -*-
  HelpPanel.h Panel for displaying controls and other help information
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef HELPPANEL_H
#define HELPPANEL_H

#include "Keyboard.h"
#include "TextPanel.h"
#include "GUIConstants.h"

namespace MFM
{

  /**
   * A Panel which displays helpful information about the driver it
   * resides in.
   */
  class HelpPanel : public TextPanel<HELP_PANEL_COLUMNS,HELP_PANEL_ROWS>
  {
  private:
    typedef TextPanel<HELP_PANEL_COLUMNS,HELP_PANEL_ROWS> Super;

    const Keyboard & m_keyboard;
    bool m_isGenerated;

    void GenerateHelpText()
    {
      Super::TextPanelByteSink & bs = Super::GetByteSink();
      bs.Reset();
      m_keyboard.PrintHelp(bs);
      m_isGenerated = true;
    }

  public:
    /**
     * Constructs a new HelpPanel that is ready to be used.
     */
    HelpPanel(const Keyboard & acceleratorMap)
      : m_keyboard(acceleratorMap)
      , m_isGenerated(false)
    {
      Panel::SetName("HelpPanel");
      Panel::SetBackground(Drawing::DARK_PURPLE);
      Panel::SetForeground(Drawing::WHITE);
    }

    virtual void SaveDetails(ByteSink& sink) const
    {
      Super::SaveDetails(sink);
      sink.Printf(" PP(isgn=%d)\n",m_isGenerated);
    }

    virtual bool LoadDetails(const char * key, LineCountingByteSource& source)
    {
      if (Super::LoadDetails(key, source)) return true;

      if (!strcmp("isgn",key)) return 1 == source.Scanf("%d",&m_isGenerated);
      return false;
    }

    virtual void PaintUpdateVisibility(Drawing& d)
    {
      if (!m_isGenerated)
      {
        GenerateHelpText();
        ScrollToTop();
      }
    }

  };
}

#endif /* HELPPANEL_H */
