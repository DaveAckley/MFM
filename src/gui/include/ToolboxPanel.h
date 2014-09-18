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
#include "FileByteSink.h"
#include "Parameters.h"
#include "AbstractButton.h"
#include "Slider.h"
#include "ParameterControllerBool.h"
#include "NeighborSelectPanel.h"

#define ELEMENT_BOX_SIZE 70
#define ELEMENT_RENDER_SIZE 32

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

    enum {
      ELEMENT_BOX_BUTTON_COUNT = 9,
      TOOLBOX_MAX_CONTROLLERS = 20,
      TOOLBOX_MAX_SLIDERS = 8,
      TOOLBOX_MAX_CHECKBOXES = 8,
      TOOLBOX_MAX_NEIGHBORHOODS = 8,

      R = CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS
    };

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
        AbstractButton(),
        m_toolboxTool(0),
        m_parent(0)
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

      EditingTool GetEditingTool()
      {
        return m_tool;
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
        // DO NOT REWRITE THIS 'IF' AS A QUESTION-COLON; IT DOES NOT
        // COMPILE ON 12.04.
        if (activated)
        {
          SetBackground(Drawing::GREY40);
        }
        else
        {
          SetBackground(Drawing::GREY80);
        }
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

        this->Panel::SetBorder(Drawing::GREY70);

        this->Panel::SetBackground(Drawing::GREY80);

        this->Panel::SetForeground(Drawing::GREY70);

        this->Panel::SetBorder(Drawing::GREY60);

        AbstractButton::SetEnabled(false);
      }

      void SetElement(Element<CC>* element)
      {
        m_element = element;

        AbstractButton::SetEnabled(!!m_element);

        if (m_element)
        {
          OString16 name;
          name.Printf("EltBtn-%s",element->GetAtomicSymbol());
          this->SetName(name.GetZString());
        }
      }

      void SetParent(ToolboxPanel<CC>* parent)
      {
        m_parent = parent;
      }

      virtual void PaintComponentNonClick(Drawing& d)
      {
        if(m_element)
        {
          d.SetForeground(m_element->PhysicsColor());
          d.FillRect(0, 0, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);
          d.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
          d.SetBackground(Drawing::BLACK);
          d.SetForeground(Drawing::WHITE);
          d.BlitBackedTextCentered(m_element->GetAtomicSymbol(),
                                   UPoint(0, 0),
                                   UPoint(ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE));
        }
        else
        {
          d.SetForeground(this->GetForeground());
          d.FillRect(0, 0, ELEMENT_RENDER_SIZE, ELEMENT_RENDER_SIZE);
        }
      }

      Element<CC>* GetElement()
      {
        return m_element;
      }

      virtual void OnClick(u8 button)
      {
        switch(button)
        {
        case SDL_BUTTON_LEFT:
          m_parent->SetPrimaryElement(m_element);
          break;
        case SDL_BUTTON_RIGHT:
          m_parent->SetSecondaryElement(m_element);
          break;
        case SDL_BUTTON_MIDDLE:
          m_element->ToggleLowlightPhysicsColor();
          break;
        default: break;
        }

        m_parent->RebuildControllers();
      }

      virtual bool OnScroll(u8 button)
      {
        return false;
      }

     private:
      Element<CC>* m_element;

      ToolboxPanel<CC>* m_parent;
    };

    EditingTool* m_toolPtr;

    ToolButton* m_activatedButton;

    ToolButton m_toolButtons[ELEMENT_BOX_BUTTON_COUNT];

    Element<CC>* m_primaryElement;

    Element<CC>* m_secondaryElement;

    Element<CC>* m_heldElements[ELEMENT_BOX_SIZE];

    ElementButton m_elementButtons[ELEMENT_BOX_SIZE];

    u32 m_heldElementCount;

    ParameterController *(m_controllers[TOOLBOX_MAX_CONTROLLERS]);
    u32 m_controllerCount;

    Slider m_sliders[TOOLBOX_MAX_SLIDERS];
    u32 m_sliderCount;

    ParameterControllerBool m_checkboxes[TOOLBOX_MAX_CHECKBOXES];
    u32 m_checkboxCount;

    NeighborSelectPanel<R> m_neighborhoods[TOOLBOX_MAX_NEIGHBORHOODS];
    u32 m_neighborhoodCount;

    u32 m_brushSize;

    enum
    {
      ELEMENTS_PER_ROW = 10,
      ELEMENT_ROWS = (ELEMENT_BOX_SIZE + ELEMENTS_PER_ROW - 1) / ELEMENTS_PER_ROW,
      NON_ELEMENT_ROWS = 3,
      TOTAL_ROWS = ELEMENT_ROWS + NON_ELEMENT_ROWS,
      BORDER_PADDING = 2
    };

    void AddController(ParameterController * spc)
    {
      MFM_API_ASSERT_NONNULL(spc);

      if (m_controllerCount >= TOOLBOX_MAX_CONTROLLERS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_controllerCount++;

      m_controllers[j] = spc;
    }

    void AddSliderController(Parameters::S32 * sp)
    {
      // Sliders for S32 parameters
      MFM_API_ASSERT_NONNULL(sp);

      if (m_sliderCount >= TOOLBOX_MAX_SLIDERS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_sliderCount++;

      Slider& s = m_sliders[j];
      s.SetParameter(sp);

      AddController(&s);
    }

    void AddCheckboxController(Parameters::Bool * bp)
    {
      // Checkboxes for Bool parameters
      MFM_API_ASSERT_NONNULL(bp);

      if (m_checkboxCount >= TOOLBOX_MAX_CHECKBOXES)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_checkboxCount++;

      ParameterControllerBool & cb = m_checkboxes[j];
      cb.SetParameter(bp);

      AddController(&cb);
    }

    void AddNeighborhoodController(Parameters::Neighborhood<R>* np)
    {
      MFM_API_ASSERT_NONNULL(np);

      if (m_neighborhoodCount >= TOOLBOX_MAX_NEIGHBORHOODS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_neighborhoodCount++;

      NeighborSelectPanel<R>& nb = m_neighborhoods[j];

      nb.SetParameter(np);
      nb.SetText(np->GetName());

      AddController(&nb);
    }

   public:

    ToolboxPanel(EditingTool* toolPtr) :
      m_toolPtr(toolPtr),
      m_activatedButton(m_toolButtons),
      m_primaryElement(&Element_Empty<CC>::THE_INSTANCE),
      m_secondaryElement(&Element_Empty<CC>::THE_INSTANCE),
      m_heldElementCount(0),
      m_controllerCount(0),
      m_sliderCount(0),
      m_checkboxCount(0),
      m_brushSize(2)
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

    void RebuildControllers()
    {
      /* Remove any old controllers */
      for(u32 i = 0; i < m_controllerCount; i++)
      {
        Panel::Remove(m_controllers[i]);
      }
      m_controllerCount = 0;

      /* Set up the new controllers, if any */
      Parameters & parms = m_primaryElement->GetElementParameters();
      u32 totalParms = parms.GetParameterCount();

      m_sliderCount = 0;
      m_checkboxCount = 0;
      m_neighborhoodCount = 0;
      for(u32 i = 0; i < totalParms; i++)
      {
        const Parameters::Parameter * parm = parms.GetParameter(i);
        switch (parm->GetParameterType())
        {
        case Parameters::S32_PARAMETER:
          AddSliderController(Parameters::S32::Cast(parms.GetParameter(i)));
          break;
        case Parameters::BOOL_PARAMETER:
          AddCheckboxController(Parameters::Bool::Cast(parms.GetParameter(i)));
          break;
        case Parameters::NEIGHBORHOOD_PARAMETER:
          AddNeighborhoodController(Parameters::Neighborhood<R>::Cast(parms.GetParameter(i)));
          break;
        default:
          FAIL(ILLEGAL_STATE);
        }
      }

      /* Put in the new controllers */
      SPoint rpt(16, 6 + ELEMENT_RENDER_SIZE * TOTAL_ROWS);
      for(u32 i = 0; i < m_controllerCount; i++)
      {
        ParameterController * c = m_controllers[i];
        c->SetRenderPoint(rpt);
        Panel::Insert(c, NULL);
        UPoint desired = c->GetDesiredSize();
        rpt.SetY(rpt.GetY() + desired.GetY());
      }

      Panel::SetDimensions(6 + ELEMENT_RENDER_SIZE * ELEMENTS_PER_ROW + BORDER_PADDING,
                           rpt.GetY() + BORDER_PADDING);
      Panel::SetDesiredSize(6 + ELEMENT_RENDER_SIZE * ELEMENTS_PER_ROW + BORDER_PADDING,
                           rpt.GetY() + BORDER_PADDING);

      //debug: this->Print(STDOUT);

      // Tell ourselfs to resize (our cheeso repacking)
      HandleResize(this->m_parent->GetDimensions());

    }

    void AddButtons()
    {
      Asset assets[] =
      {
        ASSET_SELECTOR_ICON,
        ASSET_ATOM_SELECTOR_ICON,
        ASSET_PENCIL_ICON,
        ASSET_BUCKET_ICON,
        ASSET_ERASER_ICON,
        ASSET_BRUSH_ICON,
        ASSET_XRAY_ICON,
        ASSET_CLONE_ICON,
        ASSET_AIRBRUSH_ICON
      };

      for(u32 i = 0; i < ELEMENT_BOX_BUTTON_COUNT; i++)
      {
        m_toolButtons[i].SetParent(this);
        m_toolButtons[i].Panel::SetRenderPoint(SPoint(16 + i * 32, 3));
        m_toolButtons[i].SetToolIcon(AssetManager::Get(assets[i]));
        Panel::Insert(m_toolButtons + i, NULL);
      }

      for(u32 i = 0; i < ELEMENT_BOX_SIZE; i++)
      {
        u32 x, y;

        x = 3 + ELEMENT_RENDER_SIZE * (i % ELEMENTS_PER_ROW);
        y = 3 + ELEMENT_RENDER_SIZE * (3 + (i / ELEMENTS_PER_ROW));

        /* These will render correctly because ElementButtons render
           grey when SetElement is fed NULL. */
        m_elementButtons[i].SetElement(m_heldElements[i]);
        m_elementButtons[i].SetParent(this);
        m_elementButtons[i].Panel::SetRenderPoint(SPoint(x, y));

        Panel::Insert(m_elementButtons + i, NULL);
      }

      m_primaryElement   = m_heldElements[0];
      m_secondaryElement = m_heldElements[1];

      Panel::SetDimensions(6 + ELEMENT_RENDER_SIZE * ELEMENTS_PER_ROW,
                           6 + ELEMENT_RENDER_SIZE * TOTAL_ROWS);
      Panel::SetDesiredSize(6 + ELEMENT_RENDER_SIZE * ELEMENTS_PER_ROW,
                            6 + ELEMENT_RENDER_SIZE * TOTAL_ROWS);

      m_activatedButton = m_toolButtons;
      m_toolButtons[0].ToolButton::SetActivated(true);
    }

    void SetPrimaryElement(Element<CC>* element)
    { m_primaryElement = element; }

    Element<CC>* GetPrimaryElement()
    { return m_primaryElement; }

    void SetSecondaryElement(Element<CC>* element)
    { m_secondaryElement = element; }

    u32 GetBrushSize()
    {
      if(!Panel::IsVisible())
      {
        return 0;
      }

      switch(m_activatedButton->GetEditingTool())
      {
      case TOOL_SELECTOR:
      case TOOL_ATOM_SELECTOR:
      case TOOL_PENCIL:
      case TOOL_BUCKET:
      case TOOL_ERASER:
        return 1;
      case TOOL_BRUSH:
      case TOOL_XRAY:
      case TOOL_CLONE:
      case TOOL_AIRBRUSH:
        return m_brushSize;
      default:
        FAIL(ILLEGAL_STATE);
        return 0;
      }
    }

    Element<CC>* GetSecondaryElement()
    { return m_secondaryElement; }

    void RegisterElement(Element<CC>* element)
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

    bool IsBrushableSelected()
    {
      return GetSelectedTool() == TOOL_BRUSH    ||
             GetSelectedTool() == TOOL_ERASER   ||
             GetSelectedTool() == TOOL_XRAY     ||
             GetSelectedTool() == TOOL_CLONE    ||
             GetSelectedTool() == TOOL_AIRBRUSH;
    }

    virtual bool Handle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;

      /* Only fire on wheel 'press' to avoid double-counting */
      if(event.type != SDL_MOUSEBUTTONDOWN)
      {
        return true;
      }

      if(IsBrushableSelected())
      {
        switch(event.button)
        {
        case SDL_BUTTON_WHEELUP:
          m_brushSize++;
          break;
        case SDL_BUTTON_WHEELDOWN:
          if (m_brushSize > 0)
          {
            m_brushSize--;
          }
          break;
        default:
          break;
        }
      }

      return true;
    }

    virtual bool Handle(MouseMotionEvent& mme)
    {
      /* Try to keep the grid from taking this event too */
      return true;
    }

    virtual void PaintComponent(Drawing& d)
    {
      const u32 ELEMENT_START_X = 64;
      const u32 ELEMENT_START_Y = 40;
      const u32 ELEMENT_TOOL_SIZE = ELEMENT_RENDER_SIZE + 8;

      d.SetForeground(this->Panel::GetBackground());
      d.FillRect(0, 0, this->Panel::GetWidth(), this->Panel::GetHeight());

      d.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));

      if (m_primaryElement)
      {
        d.SetForeground(m_primaryElement->PhysicsColor());
        d.FillCircle(ELEMENT_START_X, ELEMENT_START_Y, ELEMENT_TOOL_SIZE,
                     ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE / 2);

        d.SetBackground(Drawing::BLACK);
        d.SetForeground(Drawing::WHITE);
        d.BlitBackedTextCentered(m_primaryElement->GetAtomicSymbol(),
                                 UPoint(ELEMENT_START_X, ELEMENT_START_Y),
                                 UPoint(ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE));
      }
      if (m_secondaryElement)
      {
        const u32 SECONDARY_X_START = ELEMENT_START_X + ELEMENT_TOOL_SIZE;
        d.SetForeground(m_secondaryElement->PhysicsColor());
        d.FillCircle(SECONDARY_X_START, ELEMENT_START_Y, ELEMENT_TOOL_SIZE,
                     ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE / 2);

        d.SetBackground(Drawing::BLACK);
        d.SetForeground(Drawing::WHITE);
        d.BlitBackedTextCentered(m_secondaryElement->GetAtomicSymbol(),
                                 UPoint(SECONDARY_X_START, ELEMENT_START_Y),
                                 UPoint(ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE));
      }

      if(IsBrushableSelected())
      {
        d.SetBackground(Drawing::BLACK);
        d.SetForeground(Drawing::WHITE);

        char brushSizeArray[64];

        snprintf(brushSizeArray, 64, "%d", m_brushSize);

        const SPoint brushPos = m_toolButtons[TOOL_AIRBRUSH].Panel::GetRenderPoint();
        UPoint pos(brushPos.GetX() + 32, brushPos.GetY());
        d.BlitBackedText(brushSizeArray, pos, UPoint(128, 128));
      }

    }
  };
}

#endif /* TOOLBOXPANEL_H */
