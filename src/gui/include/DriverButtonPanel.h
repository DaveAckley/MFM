/*                                              -*- mode:C++ -*-
  DriverButtonPanel.h A panel for holding various driver buttons
  Copyright (C) 2014-2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file DriverButtonPanel.h A panel for holding various driver buttons
  \author Trent R. Small.
  \author Dave Ackley
  \date (C) 2014, 2016 All rights reserved.
  \lgpl
*/
#ifndef DRIVERBUTTONPANEL_H
#define DRIVERBUTTONPANEL_H

#include "AbstractButton.h"
#include "GUIConstants.h"
#include "MovablePanel.h"

namespace MFM
{
  template <class GC> class Grid;    // FORWARD
  template <class GC> class GridPanel;    // FORWARD

  struct DriverButtonPanel : public MovablePanel, public KeyboardCommandFunction
  {
    virtual s32 GetSection() { return HELP_SECTION_WINDOWS; }

    virtual const char * GetDoc() { return "Toggle showing the button window"; }

    virtual bool GetKey(u32& keysym, u32& mods)
    {
      keysym = SDLK_b;
      mods = KMOD_CTRL;
      return true;
    }

    DriverButtonPanel* m_otherDriverButtonPanel;

    struct OtherButtonPanelButton : AbstractButton 
    {
      DriverButtonPanel & m_parent;

      OtherButtonPanelButton(const char * panelName,
                             const char * text, 
                             DriverButtonPanel & parent)
        : AbstractButton(text)
        , m_parent(parent)
      { 
        SetName(panelName);
      }

      virtual void OnClick(u8 button) 
      {
        m_parent.ShowOtherDriverButtonPanel();
      }

      virtual bool ExecuteFunction(u32 keysym, u32 mods) { MFM_API_ASSERT_STATE(0); }
      virtual s32 GetSection() { MFM_API_ASSERT_STATE(0); }
      virtual const char * GetDoc() { MFM_API_ASSERT_STATE(0); }
      virtual bool GetKey(u32& keysym, u32& mods) { MFM_API_ASSERT_STATE(0); }

    } m_otherDriverButtonPanelButton;

    void SetOtherDriverButtonPanel(DriverButtonPanel & other) 
    {
      m_otherDriverButtonPanel = &other;
    }

    void ShowOtherDriverButtonPanel()
    {
      MFM_API_ASSERT_STATE(m_otherDriverButtonPanel);
      this->SetVisible(false);
      m_otherDriverButtonPanel->SetVisible(true);
    }

    virtual bool ExecuteFunction(u32 keysym, u32 mods)
    {
      bool was = this->IsVisible();
      this->SetVisible(!was);
      if (m_otherDriverButtonPanel)
        m_otherDriverButtonPanel->SetVisible(was);

      return true;
    }

    static const u32 BUTTON_SPACING_HEIGHT = 34;

    virtual void PaintBorder(Drawing & config)
    { /* No border please */ }

    DriverButtonPanel(const char * name, bool isMore)
      : m_otherDriverButtonPanel(0)
      , m_otherDriverButtonPanelButton(isMore?"more":"less",
                                 isMore?"More>>":"<<Less",
                                 *this)
    {
      SetName(name);
      SetDimensions(STATS_START_WINDOW_WIDTH,
                    SCREEN_INITIAL_HEIGHT / 2);
      SetDesiredSize(STATS_START_WINDOW_WIDTH, SCREEN_INITIAL_HEIGHT / 2);
      SetAnchor(ANCHOR_SOUTH);
      SetAnchor(ANCHOR_EAST);
      SetForeground(Drawing::WHITE);
      SetBackground(Drawing::LIGHTER_DARK_PURPLE);
      SetFont(FONT_ASSET_ELEMENT);

      m_otherDriverButtonPanelButton.SetRenderPoint(SPoint(10, 10));
      m_otherDriverButtonPanelButton.Panel::SetDimensions(100, 30);
      Panel::Insert(&m_otherDriverButtonPanelButton, 0);
    }

    void InsertAndPlace(Panel & b)
    {
      u32 kids = GetChildCount(); // quadratic!
      b.SetRenderPoint(SPoint(2, 2 + kids * BUTTON_SPACING_HEIGHT));
      b.Panel::SetDimensions(STATS_WINDOW_WIDTH, BUTTON_SPACING_HEIGHT);
      Panel::Insert(&b, 0);
    }

  private:

  };
}
#endif /* DRIVERBUTTONPANEL_H */
