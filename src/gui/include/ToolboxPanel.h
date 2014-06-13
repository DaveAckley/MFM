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

#define ELEMENT_BOX_HEIGHT 10
#define ELEMENT_BOX_WIDTH 5
#define ELEMENT_BOX_SIZE (ELEMENT_BOX_WIDTH * ELEMENT_BOX_HEIGHT)
#define ELEMENT_RENDER_SIZE 15

namespace MFM
{
  template<class CC>
  class ToolboxPanel : public Panel
  {
  private:

    class AbstractToolButton : public AbstractButton
    {
    protected:
      EditingTool* m_toolboxTool;

      bool m_activated;

      ToolboxPanel<CC>* m_parent;

    public:

      AbstractToolButton(EditingTool* toolboxTool) :
	AbstractButton(), m_toolboxTool(toolboxTool)
      {
	this->Panel::SetBackground(Drawing::GREY80);
	this->Panel::SetBorder(Drawing::GREY40);
      }

      void SetParent(ToolboxPanel<CC>* parent)
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

    struct SelectorButton : public AbstractToolButton
    {
      SelectorButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*(this->AbstractToolButton::m_toolboxTool) = TOOL_SELECTOR;
	(this->AbstractToolButton::m_parent)->ActivateButton(this);
      }
    }m_selectorButton;

    struct PencilButton : public AbstractToolButton
    {
      PencilButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*(this->AbstractToolButton::m_toolboxTool) = TOOL_PENCIL;
	(this->AbstractToolButton::m_parent)->ActivateButton(this);
      }
    }m_pencilButton;

    struct BucketButton : public AbstractToolButton
    {
      BucketButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*(this->AbstractToolButton::m_toolboxTool) = TOOL_BUCKET;
	(this->AbstractToolButton::m_parent)->ActivateButton(this);
      }
    }m_bucketButton;

    struct EraserButton : public AbstractToolButton
    {
      EraserButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*(this->AbstractToolButton::m_toolboxTool) = TOOL_ERASER;
	(this->AbstractToolButton::m_parent)->ActivateButton(this);
      }
    }m_eraserButton;


    struct BrushButton : public AbstractToolButton
    {
      BrushButton(EditingTool* toolboxTool) :
	AbstractToolButton(toolboxTool)
      { }

      virtual void OnClick()
      {
	*(this->AbstractToolButton::m_toolboxTool) = TOOL_BRUSH;
	(this->AbstractToolButton::m_parent)->ActivateButton(this);
      }
    }m_brushButton;

    struct ElementButton : public AbstractButton
    {
      ElementButton() :
	AbstractButton()
      {
	this->Panel::SetDimensions(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);

	this->Panel::SetBackground(Drawing::GREY80);
      }

      void SetElement(const Element<CC>* element)
      {
	m_element = element;
      }

      void SetParent(ToolboxPanel<CC>* parent)
      {
	m_parent = parent;
      }

      virtual void PaintComponent(Drawing& d)
      {
	d.SetForeground(this->Panel::GetBackground());
	d.FillRect(0, 0, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);
	d.SetForeground(m_element->DefaultPhysicsColor());
	d.FillCircle(0, 0,
		     ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE,
		     ELEMENT_RENDER_SIZE >> 1);
      }

      const Element<CC>* GetElement()
      {
	return m_element;
      }

      virtual void OnClick()
      {
	m_parent->SetPrimaryElement(m_element);
	m_parent->SelectElementButton(this);
      }

      void SetActivated(bool activated)
      {
	SetBackground(activated ? Drawing::GREY40 : Drawing::GREY80);
      }

    private:
      const Element<CC>* m_element;

      ToolboxPanel<CC>* m_parent;
    };

    EditingTool* m_toolPtr;

    AbstractToolButton* m_activatedButton;

    ElementButton* m_selectedElementButton;

    const Element<CC>* m_primaryElement;

    const Element<CC>* m_secondaryElement;

    const Element<CC>* m_heldElements[ELEMENT_BOX_SIZE];

    ElementButton m_elementButtons[ELEMENT_BOX_SIZE];

    u32 m_heldElementCount;

    s32 m_selectedElementDrawY;

  public:

    ToolboxPanel(EditingTool* toolPtr) :
      m_selectorButton(toolPtr),
      m_pencilButton(toolPtr),
      m_bucketButton(toolPtr),
      m_eraserButton(toolPtr),
      m_brushButton(toolPtr),
      m_toolPtr(toolPtr),
      m_activatedButton(&m_selectorButton),
      m_selectedElementButton(NULL),
      m_primaryElement(NULL),
      m_secondaryElement(NULL),
      m_heldElementCount(0),
      m_selectedElementDrawY(0)
    { }

    void AddButtons()
    {
      AbstractToolButton* buttons[] =
      {
	&m_selectorButton,
	&m_pencilButton,
	&m_bucketButton,
	&m_eraserButton,
	&m_brushButton
      };

      Asset assets[] =
      {
	ASSET_SELECTOR_ICON,
	ASSET_PENCIL_ICON,
	ASSET_BUCKET_ICON,
	ASSET_ERASER_ICON,
	ASSET_BRUSH_ICON
      };

      u32 buttonCount = 5;
      u32 x, y;
      for(y = 0; y < 10; y++)
      {
	for(x = 0; x < 2; x++)
	{
	  u32 i = y * 2 + x;
	  if(i >= buttonCount)
	  {
	    /* semi-clean multiloop break using goto */
	    goto toolboxpanel_addbuttons_toolbuttons_loopend;
	  }

	  buttons[i]->SetParent(this);

	  buttons[i]->Panel::SetRenderPoint(SPoint(5 + x * 37, 5 + y * 37));
	  buttons[i]->SetToolIcon(AssetManager::Get(assets[i]));
	  this->Panel::Insert(buttons[i], NULL);
	  LOG.Debug("Selector(%d, %d, %d, %d)",
		    buttons[i]->GetAbsoluteLocation().GetX(),
		    buttons[i]->GetAbsoluteLocation().GetY(),
		    buttons[i]->GetDimensions().GetX(),
		    buttons[i]->GetDimensions().GetY());
	}
      }
    toolboxpanel_addbuttons_toolbuttons_loopend:

      SPoint currentDimensions(5 + 2 * 37,
			       5 + (y + ((x & 1) ? 1 : 0)) * 37 + 2 * ELEMENT_RENDER_SIZE);

      for(y = 0; y < ELEMENT_BOX_HEIGHT; y++)
      {
	for(x = 0; x < ELEMENT_BOX_WIDTH; x++)
	{
	  u32 i = y * ELEMENT_BOX_WIDTH + x;

	  if(i >= m_heldElementCount)
	  {
	    goto toolboxpanel_addbuttons_elemenbuttons_loopend;
	  }

	  m_elementButtons[i].SetElement(m_heldElements[i]);
	  m_elementButtons[i].SetParent(this);
	  m_elementButtons[i].Panel::SetRenderPoint(SPoint(4 + x * ELEMENT_RENDER_SIZE - 1,
							   currentDimensions.GetY() +
						           y * ELEMENT_RENDER_SIZE - 1));
	  this->Panel::Insert(m_elementButtons + i, NULL);
	}
      }

    toolboxpanel_addbuttons_elemenbuttons_loopend:

      m_selectedElementDrawY = currentDimensions.GetY() - 2 * ELEMENT_RENDER_SIZE;

      m_primaryElement   = m_heldElements[0];
      m_secondaryElement = m_heldElements[1];

      currentDimensions.Add(0, y * ELEMENT_RENDER_SIZE + 32);

      /* Set up the correct dimensions. X is always constant, as there
       * will (hopefully) always be two tools. Y is more complicated,
       * since we need to increase it if there are an odd number of tools. */
      this->Panel::SetDimensions(currentDimensions.GetX(), currentDimensions.GetY());

      m_activatedButton = buttons[0];
      buttons[0]->AbstractToolButton::SetActivated(true);
    }

    void SetPrimaryElement(const Element<CC>* element)
    { m_primaryElement = element; }

    const Element<CC>* GetPrimaryElement()
    { return m_primaryElement; }

    void SetSecondaryElement(const Element<CC>* element)
    { m_secondaryElement = element; }

    const Element<CC>* GetSecondaryElement()
    { return m_secondaryElement; }

    void SelectElementButton(ElementButton* button)
    {
      if(m_selectedElementButton)
      {
	m_selectedElementButton->SetActivated(false);
      }
      m_selectedElementButton = button;
      button->SetActivated(true);
    }

    void RegisterElement(const Element<CC>* element)
    {
      if(m_heldElementCount >= ELEMENT_BOX_SIZE)
      {
	FAIL(OUT_OF_ROOM);
      }
      m_heldElements[m_heldElementCount++] = element;
    }

    void ActivateButton(AbstractToolButton* button)
    {
      if(m_activatedButton)
      {
	m_activatedButton->SetActivated(false);
      }
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

    virtual void PaintComponent(Drawing& d)
    {
      d.SetForeground(this->Panel::GetBackground());
      d.FillRect(0, 0, this->Panel::GetWidth(), this->Panel::GetHeight());

      d.SetForeground(m_primaryElement->DefaultPhysicsColor());
      d.FillCircle(10 + ELEMENT_RENDER_SIZE, m_selectedElementDrawY, ELEMENT_RENDER_SIZE,
		   ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE / 2);
      d.SetForeground(m_secondaryElement->DefaultPhysicsColor());
      d.FillCircle(10 + 2 * ELEMENT_RENDER_SIZE, m_selectedElementDrawY,
		   ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE / 2);
    }
  };
}

#endif /* TOOLBOXPANEL_H */
