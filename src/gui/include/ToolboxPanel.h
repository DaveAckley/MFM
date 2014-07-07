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

#define ELEMENT_BOX_SIZE 72
#define ELEMENT_RENDER_SIZE 20

#define ELEMENT_BOX_BUTTON_COUNT 5

namespace MFM
{
  /**
   * A class representing the Panel which allows a user to select from
   * a collection of Element drawing tools.
   */
  template<class CC>
  class ToolboxPanel : public Panel
  {
  private:

    /**
     * A class representing a Button for selecting a particular
     * EditingTool.
     */
    class ToolButton : public AbstractButton
    {
    protected:
      EditingTool* m_toolboxTool;

      ToolboxPanel<CC>* m_parent;

      EditingTool m_tool;

    public:

      /**
       * Construct a new ToolButton, pointing at a specified
       * EditingTool location.
       *
       * @param toolboxTool An EditingTool pointer, the contents of
       * which should be modified upon clicking this Button.
       */
      ToolButton() :
	AbstractButton()
      {
	this->Panel::SetBackground(Drawing::GREY90);
	this->Panel::SetBorder(Drawing::GREY40);
      }

      void SetToolPointer(EditingTool* toolboxTool)
      {
	m_toolboxTool = toolboxTool;
      }

      void SetEditingTool(EditingTool tool)
      {
	m_tool = tool;
      }

      /**
       * Sets the ToolboxPanel which owns this
       * ToolButton. This is required before clicking any
       * ToolButton.
       */
      void SetParent(ToolboxPanel<CC>* parent)
      {
	m_parent = parent;
      }

      /**
       * Sets the icon of this ToolButton to a particular
       * SDL_Surface*, which will be rendered when needed.
       *
       * @param icon The SDL_Surface* which represents this
       * ToolButton's icon.
       */
      void SetToolIcon(SDL_Surface* icon)
      {
	SetIcon(icon);
	this->Panel::SetDimensions(icon->w, icon->h);
      }

      /**
       * Sets the visual properties of this ToolButton to
       * appear that it is activated or deactivated.
       *
       * @param activated If \c true, this ToolButton will
       *                  appear active. If not, it will appear
       *                  disabled.
       */
      void SetActivated(bool activated)
      {
        if (activated)
          SetBackground(Drawing::GREY40);
        else
          SetBackground(Drawing::GREY80);
      }

      /**
       * This hook is invoked when a user clicks on this button,
       * therefore setting the active tool of this ToolButton.
       *
       * @param button The button on the mouse that was pressed.
       */
      virtual void OnClick(u8 button)
      {
	*m_toolboxTool = m_tool;
	m_parent->ActivateButton(this);
      }
    };

    struct ElementButton : public AbstractButton
    {
      ElementButton() :
	AbstractButton()
      {
	this->Panel::SetDimensions(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);

	this->Panel::SetBackground(Drawing::GREY80);

	AbstractButton::SetEnabled(false);
      }

      void SetElement(const Element<CC>* element)
      {
	m_element = element;

	AbstractButton::SetEnabled(!!m_element);
      }

      void SetParent(ToolboxPanel<CC>* parent)
      {
	m_parent = parent;
      }

      virtual void PaintComponent(Drawing& d)
      {
	if(m_element)
	{
	  d.SetForeground(m_element->DefaultPhysicsColor());
	  d.FillRect(0, 0, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);
	  d.SetFont(AssetManager::GetFont(FONT_ASSET_ELEMENT));

	  d.SetForeground(Drawing::HalfColor(~m_element->DefaultPhysicsColor()));
	  d.BlitText(m_element->GetAtomicSymbol(),
		     UPoint(3, 1),
		     UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));

	  d.SetForeground(~m_element->DefaultPhysicsColor());
	  d.BlitText(m_element->GetAtomicSymbol(),
		     UPoint(2, 0),
		     UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));
	}
	else
	{
	  d.SetForeground(Drawing::GREY80);
	  d.FillRect(0, 0, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);
	}
      }

      const Element<CC>* GetElement()
      {
	return m_element;
      }

      virtual void OnClick(u8 button)
      {
	if(button == SDL_BUTTON_LEFT)
	{
	  m_parent->SetPrimaryElement(m_element);
	}
	else
	{
	  m_parent->SetSecondaryElement(m_element);
	}
      }

    private:
      const Element<CC>* m_element;

      ToolboxPanel<CC>* m_parent;
    };

    EditingTool* m_toolPtr;

    ToolButton* m_activatedButton;

    ToolButton m_toolButtons[ELEMENT_BOX_BUTTON_COUNT];

    const Element<CC>* m_primaryElement;

    const Element<CC>* m_secondaryElement;

    const Element<CC>* m_heldElements[ELEMENT_BOX_SIZE];

    ElementButton m_elementButtons[ELEMENT_BOX_SIZE];

    u32 m_heldElementCount;

  public:

    ToolboxPanel(EditingTool* toolPtr) :
      m_toolPtr(toolPtr),
      m_activatedButton(m_toolButtons),
      m_primaryElement(NULL),
      m_secondaryElement(NULL),
      m_heldElementCount(0)
    {
      for(u32 i = 0; i < ELEMENT_BOX_BUTTON_COUNT; i++)
      {
	m_toolButtons[i].SetToolPointer(m_toolPtr);
	m_toolButtons[i].SetEditingTool((EditingTool)i);
      }

      for(u32 i = 0; i < ELEMENT_BOX_SIZE; i++)
      {
	m_heldElements[i] = NULL;
      }
    }

    void AddButtons()
    {
      Asset assets[] =
      {
	ASSET_SELECTOR_ICON,
	ASSET_PENCIL_ICON,
	ASSET_BUCKET_ICON,
	ASSET_ERASER_ICON,
	ASSET_BRUSH_ICON
      };

      for(u32 i = 0; i < ELEMENT_BOX_BUTTON_COUNT; i++)
      {
	m_toolButtons[i].SetParent(this);

	m_toolButtons[i].Panel::SetRenderPoint(SPoint(69 + i * 32, 3));
	m_toolButtons[i].SetToolIcon(AssetManager::Get(assets[i]));
	Panel::Insert(m_toolButtons + i, NULL);
	LOG.Debug("Selector(%d, %d, %d, %d)",
		  m_toolButtons[i].GetAbsoluteLocation().GetX(),
		  m_toolButtons[i].GetAbsoluteLocation().GetY(),
		  m_toolButtons[i].GetDimensions().GetX(),
		  m_toolButtons[i].GetDimensions().GetY());
      }

      /* This is a little hairy, but I'm going to attempt rendering
       * like the periodic table. */
      for(u32 i = 0; i < ELEMENT_BOX_SIZE; i++)
      {
	u32 x, y;
	if(i < 18)
	{
	  /* H, Li, Na */
	  x = 3;
	  y = 3;
	  if(i == 1)
	  {
	    /* He */
	    x += ELEMENT_RENDER_SIZE * 17;
	  }
	  else if(i == 3 || i == 0xb)
	  {
	    /*Be, Mg*/
	    x += ELEMENT_RENDER_SIZE;
	  }
	  else if(i >= 4 && i <= 9)
	  {
	    /*B - Ne */
	    x += ELEMENT_RENDER_SIZE * (12 + (i - 4));
	  }
	  else if(i >= 0xc && i <= 0x11)
	  {
	    /* Al - Ar*/
	    x += ELEMENT_RENDER_SIZE * (12 + (i - 0xc));
	  }

	  if(i >= 2 && i <= 9)
	  {
	    y += ELEMENT_RENDER_SIZE;
	  }
	  else if(i >= 0xa)
	  {
	    y += 2 * ELEMENT_RENDER_SIZE;
	  }
	}
	else
	{
	  /* From this point on it's grid rendering */
	  x = 3 + ELEMENT_RENDER_SIZE * (i % 18);
	  y = 3 + ELEMENT_RENDER_SIZE * (2 + (i / 18));
	}


	/* These will render correctly because ElementButtons render
	   grey when SetElement is fed NULL. */
	m_elementButtons[i].SetElement(m_heldElements[i]);
	m_elementButtons[i].SetParent(this);
	m_elementButtons[i].Panel::SetRenderPoint(SPoint(x, y));

	Panel::Insert(m_elementButtons + i, NULL);
      }

      m_primaryElement   = m_heldElements[0];
      m_secondaryElement = m_heldElements[1];

      Panel::SetDimensions(6 + ELEMENT_RENDER_SIZE * 18,
	                   6 + ELEMENT_RENDER_SIZE * 6);

      m_activatedButton = m_toolButtons;
      m_toolButtons[0].ToolButton::SetActivated(true);
    }

    void SetPrimaryElement(const Element<CC>* element)
    { m_primaryElement = element; }

    const Element<CC>* GetPrimaryElement()
    { return m_primaryElement; }

    void SetSecondaryElement(const Element<CC>* element)
    { m_secondaryElement = element; }

    const Element<CC>* GetSecondaryElement()
    { return m_secondaryElement; }

    void RegisterElement(const Element<CC>* element)
    {
      if(m_heldElementCount >= ELEMENT_BOX_SIZE)
      {
	FAIL(OUT_OF_ROOM);
      }
      m_heldElements[m_heldElementCount++] = element;
    }

    void ActivateButton(ToolButton* button)
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

      d.SetFont(AssetManager::GetFont(FONT_ASSET_ELEMENT));



      if (m_primaryElement) {
        d.SetForeground(m_primaryElement->DefaultPhysicsColor());
        d.FillCircle(129, 40, ELEMENT_RENDER_SIZE,
                     ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE / 2);

	d.SetForeground(Drawing::HalfColor(~this->Panel::GetBackground()));
	d.BlitText(m_primaryElement->GetAtomicSymbol(),
		   UPoint(129 - ELEMENT_RENDER_SIZE , 41),
		   UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));

	d.SetForeground(~this->Panel::GetBackground());
	d.BlitText(m_primaryElement->GetAtomicSymbol(),
		   UPoint(128 - ELEMENT_RENDER_SIZE, 40),
		   UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));
      }
      if (m_secondaryElement) {
        d.SetForeground(m_secondaryElement->DefaultPhysicsColor());
        d.FillCircle(129 + ELEMENT_RENDER_SIZE, 40, ELEMENT_RENDER_SIZE,
		     ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE / 2);

	d.SetForeground(Drawing::HalfColor(~this->Panel::GetBackground()));
	d.BlitText(m_secondaryElement->GetAtomicSymbol(),
		   UPoint(129 + ELEMENT_RENDER_SIZE * 2, 41),
		   UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));

	d.SetForeground(~this->Panel::GetBackground());
	d.BlitText(m_secondaryElement->GetAtomicSymbol(),
		   UPoint(128 + ELEMENT_RENDER_SIZE * 2, 40),
		   UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));
      }
    }
  };
}

#endif /* TOOLBOXPANEL_H */
