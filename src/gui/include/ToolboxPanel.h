/*                                              -*- mode:C++ -*-
  ToolboxPanel.h A Panel for selecting editing tools
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
  \file ToolboxPanel.h A Panel for selecting editing tools
  \author Trent R. Small
  \date (C) 2014 All rights reserved.
  \lgpl
*/
#ifndef TOOLBOXPANEL_H
#define TOOLBOXPANEL_H

#include "EditingTool.h"
#include "AbstractButton.h"

namespace MFM
{

  class ToolboxPanel; /* Forward Declaration */

  class AbstractToolButton : public AbstractButton
  {
  protected:
    EditingTool* m_toolboxTool;

    bool m_activated;

    ToolboxPanel* m_parent;

  public:

    AbstractToolButton(EditingTool* toolboxTool) :
      AbstractButton(), m_toolboxTool(toolboxTool)
    {
      this->Panel::SetBackground(Drawing::GREY80);
      this->Panel::SetBorder(Drawing::GREY40);
    }

    void SetParent(ToolboxPanel* parent)
    {
      m_parent = parent;
    }

    bool IsActivated()
    {
      return m_activated;
    }

    void SetToolIcon(SDL_Surface* icon)
    {
      SetIcon(icon);
      this->Panel::SetDimensions(icon->w, icon->h);
    }

    void SetActivated(bool activated)
    {
      m_activated = activated;
      SetBackground(activated ? Drawing::GREY40 : Drawing::GREY80);
    }
  };

  class ToolboxPanel : public Panel
  {
  private:

    EditingTool* m_toolPtr;

    AbstractToolButton* m_activatedButton;

    struct SelectorButton : public AbstractToolButton
    {
      SelectorButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*m_toolboxTool = TOOL_SELECTOR;
	m_parent->ActivateButton(this);
      }
    }m_selectorButton;

    struct PencilButton : public AbstractToolButton
    {
      PencilButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*m_toolboxTool = TOOL_PENCIL;
	m_parent->ActivateButton(this);
      }
    }m_pencilButton;


    struct EraserButton : public AbstractToolButton
    {
      EraserButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*m_toolboxTool = TOOL_ERASER;
	m_parent->ActivateButton(this);
      }
    }m_eraserButton;


    struct BrushButton : public AbstractToolButton
    {
      BrushButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*m_toolboxTool = TOOL_BRUSH;
	m_parent->ActivateButton(this);
      }
    }m_brushButton;

  public:

    ToolboxPanel(EditingTool* toolPtr) :
      m_toolPtr(toolPtr),
      m_activatedButton(&m_selectorButton),
      m_selectorButton(toolPtr),
      m_pencilButton(toolPtr),
      m_eraserButton(toolPtr),
      m_brushButton(toolPtr)
    { }

    void AddButtons()
    {
      AbstractToolButton* buttons[] =
      {
	&m_selectorButton,
	&m_pencilButton,
	&m_eraserButton,
	&m_brushButton
      };

      Asset assets[] =
      {
	ASSET_SELECTOR_ICON,
	ASSET_PENCIL_ICON,
	ASSET_ERASER_ICON,
	ASSET_BRUSH_ICON
      };

      u32 buttonCount = 4;
      u32 x, y;
      for(y = 0; y < 10; y++)
      {
	for(x = 0; x < 2; x++)
	{
	  u32 i = y * 2 + x;
	  if(i >= buttonCount)
	  {
	    /* semi-clean multiloop break using goto */
	    goto toolboxpanel_addbuttons_loopend;
	  }

	  buttons[i]->SetParent(this);

	  buttons[i]->Panel::SetRenderPoint(SPoint(5 + x * 37, 5 + y * 37));
	  buttons[i]->SetToolIcon(AssetManager::Get(assets[i]));
	  this->Insert(buttons[i], NULL);
	  LOG.Debug("Selector(%d, %d, %d, %d)",
		    buttons[i]->GetAbsoluteLocation().GetX(),
		    buttons[i]->GetAbsoluteLocation().GetY(),
		    buttons[i]->GetDimensions().GetX(),
		    buttons[i]->GetDimensions().GetY());
	}
      }
    toolboxpanel_addbuttons_loopend:

      this->Panel::SetDimensions(5 + x * 32, 5 + y * 37);

      m_activatedButton = buttons[0];
      buttons[0]->AbstractToolButton::SetActivated(true);
    }

    void ActivateButton(AbstractToolButton* button)
    {
      m_activatedButton->SetActivated(false);
      m_activatedButton = button;
      button->SetActivated(true);
    }

    EditingTool GetSelectedTool()
    {
      return *m_toolPtr;
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      /* Try to keep the grid from taking this event too */
      return true;
    }
  };
}

#endif /* TOOLBOXPANEL_H */
