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

namespace MFM
{
  enum EditingTool
  {
    TOOL_SELECTOR,
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_BRUSH
  };

  class AbstractToolButton : public AbstractButton
  {
  protected:
    EditingTool* m_toolboxTool;

  public:

    AbstractToolButton(EditingTool* toolboxTool) :
      AbstractButton()
    {
      m_toolboxTool = toolboxTool;
      this->Panel::SetBackground(Drawing::GREY80);
      this->Panel::SetBorder(Drawing::GREY40);
    }

    void SetToolIcon(SDL_Surface* icon)
    {
      SetIcon(icon);
      this->Panel::SetDimensions(icon->w, icon->h);
    }

  };

  struct SelectorButton : public AbstractToolButton
  {
  public:

    SelectorButton(EditingTool* toolboxTool) :
      AbstractToolButton(toolboxTool)
    { }

    virtual void OnClick()
    {
      *m_toolboxTool = TOOL_SELECTOR;
    }
  };

  struct PencilButton : public AbstractToolButton
  {
  public:

    PencilButton(EditingTool* toolboxTool) :
      AbstractToolButton(toolboxTool)
    { }

    virtual void OnClick()
    {
      *m_toolboxTool = TOOL_PENCIL;
    }
  };


  struct EraserButton : public AbstractToolButton
  {
  public:

    EraserButton(EditingTool* toolboxTool) :
      AbstractToolButton(toolboxTool)
    { }

    virtual void OnClick()
    {
      *m_toolboxTool = TOOL_ERASER;
    }
  };


  struct BrushButton : public AbstractToolButton
  {
  public:

    BrushButton(EditingTool* toolboxTool) :
      AbstractToolButton(toolboxTool)
    { }

    virtual void OnClick()
    {
      *m_toolboxTool = TOOL_BRUSH;
    }
  };

  class ToolboxPanel : public Panel
  {
  private:

    EditingTool m_selectedTool;

    SelectorButton m_selectorButton;

    PencilButton m_pencilButton;

    EraserButton m_eraserButton;

    BrushButton m_brushButton;

  public:

    ToolboxPanel() :
      m_selectedTool(TOOL_SELECTOR),
      m_selectorButton(&m_selectedTool),
      m_pencilButton(&m_selectedTool),
      m_eraserButton(&m_selectedTool),
      m_brushButton(&m_selectedTool)
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
	    goto addbutton_loopend; /* goto for multibreak */
	  }

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
    addbutton_loopend:

      this->Panel::SetDimensions(5 + x * 32, 5 + y * 37);
    }

    EditingTool GetSelectedTool()
    {
      return m_selectedTool;
    }
  };
}

#endif /* TOOLBOXPANEL_H */
