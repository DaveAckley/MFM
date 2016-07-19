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
  \author Dave Ackley
  \date (C) 2014, 2016 All rights reserved.
  \lgpl
*/
#ifndef TOOLBOXPANEL_H
#define TOOLBOXPANEL_H

#include "GridTool.h"
#include "FileByteSink.h"
#include "Parameter.h"
#include "AbstractButton.h"
#include "Slider.h"
#include "ParameterControllerBool.h"
#include "NeighborSelectPanel.h"
#include "MovablePanel.h"

namespace MFM
{
  /**
   * A class representing the Panel which allows a user to select from
   * a collection of Element drawing tools.
   */
  template<class GC>
  class ToolboxPanel : public MovablePanel
  {
  private:
    typedef MovablePanel Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    typedef ElementParameterS32<EC> OurParameterS32; // ToolboxPanel controls Elements (vs AtomViewPanel)
    typedef ElementParameterBool<EC> OurParameterBool; // ToolboxPanel controls Elements (vs AtomViewPanel)
    typedef Slider<EC> OurSlider;
    typedef ParameterControllerBool<EC> OurParameterControllerBool;

    enum {
      R = EC::EVENT_WINDOW_RADIUS,
      SITES = EVENT_WINDOW_SITES(R)
    };

    const char * m_hoverElementLabel;
    SPoint m_hoverElementLabelAt;

    bool m_bigText;

    bool m_siteEdit;

    inline u32 GetElementRenderSize()
    {
      return m_bigText ? ELEMENT_BIG_RENDER_SIZE : ELEMENT_RENDER_SIZE;
    }

    inline FontAsset GetElementRenderFont()
    {
      return m_bigText ? FONT_ASSET_ELEMENT_BIG : FONT_ASSET_ELEMENT;
    }

    class SiteEditorCheckbox : public AbstractCheckbox
    {
    private:
      ToolboxPanel<GC> & m_toolboxPanel;
      u32 m_oldBackDraw;

    public:
      SiteEditorCheckbox(ToolboxPanel<GC> & tbp)
        : AbstractCheckbox("Edit bases")
        , m_toolboxPanel(tbp)
      {
        AbstractButton::SetName("BaseEdit");
        Panel::SetDoc("Edit bases rather than atoms");
        Panel::SetFont(FONT_ASSET_BUTTON_MEDIUM);

        this->SetEnabledBg(Drawing::GREY60);
        this->SetEnabledFg(Drawing::BLACK);
      }

      virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mod)
      {
        keysym = SDLK_e;
        mod = KMOD_CTRL;
        return true;
      }

      virtual bool IsChecked() const
      {
        return m_toolboxPanel.IsSiteEdit();
      }

      virtual void OnCheck(bool checked)
      {
        this->SetChecked(checked);
      }

      virtual void SetChecked(bool checked)
      {
        m_toolboxPanel.SetSiteEdit(checked);
      }

    };

    /**
     * A class representing a Button for selecting a particular
     * GridTool.
     */
    class ToolButton : public AbstractButton
    {
    private:
      GridToolShapeUpdater<GC> * m_gridTool;
      ToolboxPanel<GC>* m_toolbox;
      s32 m_keysym;
      u32 m_mods;

     public:

      virtual s32 GetSection() { return HELP_SECTION_EDITING; }
      virtual const char * GetDoc() { return Panel::GetDoc(); }
      virtual bool GetKey(u32& keysym, u32& mods)
      {
        if (m_keysym < 0)
          return false;
        keysym = m_keysym;
        mods = m_mods;
        return true;
      }

      virtual bool ExecuteFunction(u32 keysym, u32 mods) {
        if (m_keysym >= 0 && keysym == (u32) m_keysym && mods == mods)
        {
          OnClick(SDL_BUTTON_LEFT);
          return true;
        }
        return false;
      }

      /**
       * Construct a new ToolButton
       */
      ToolButton()
        : AbstractButton()
        , m_gridTool(0)
        , m_toolbox(0)
        , m_keysym(-1)
        , m_mods(0)
      {
        this->Panel::SetBackground(Drawing::GREY40);
        this->Panel::SetBorder(Drawing::GREY40);
      }

      void SetGridTool(GridToolShapeUpdater<GC>& gridTool)
      {
        m_gridTool = &gridTool;
        SetIconSlot(GetGridTool().GetIconSlot());
      }

      void SetKeysym(s32 key, u32 mods)
      {
        m_keysym = key;
        m_mods = mods;
      }

      GridToolShapeUpdater<GC> & GetGridTool()
      {
        MFM_API_ASSERT_NONNULL(m_gridTool);
        return *m_gridTool;
      }

      const GridToolShapeUpdater<GC> & GetGridTool() const
      {
        MFM_API_ASSERT_NONNULL(m_gridTool);
        return *m_gridTool;
      }

      /**
       * Sets the ToolboxPanel which owns this
       * ToolButton. This is required before clicking any
       * ToolButton.
       */
      void SetToolbox(ToolboxPanel<GC>& parent)
      {
        m_toolbox = &parent;
      }

      ToolboxPanel<GC>& GetToolbox()
      {
        MFM_API_ASSERT_NONNULL(m_toolbox);
        return *m_toolbox;
      }

      /**
       * Tell this button it is or isn't the selected tool
       *
       * @param selected If \c true, this ToolButton will
       *                  appear active. If not, it will appear
       *                  disabled.
       */
      void SetSelected(bool selected)
      {
        // DO NOT REWRITE THIS 'IF' AS A QUESTION-COLON; IT DOES NOT
        // COMPILE ON 12.04.
        if (selected)
        {
          SetEnabledBg(Drawing::GREY40);
          this->GetGridTool().Selected();
        }
        else
        {
          SetEnabledBg(Drawing::GREY80);
          this->GetGridTool().Deselected();
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
        this->GetToolbox().SelectToolButton(*this);
      }
    };

    /**
     * A class representing a Button for selecting a particular
     * shape for the current GridTool.
     */
    class ShapeButton : public AbstractButton
    {
    private:
      ToolboxPanel<GC>& m_toolbox;
      const ToolShape m_shape;

     public:
      virtual s32 GetSection() { return HELP_SECTION_EDITING; }
      virtual const char * GetDoc() { return this->Panel::GetDoc(); }
      virtual bool GetKey(u32& keysym, u32& mods) { return false; }
      virtual bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

      /**
       * Construct a new ShapeButton
       */
      ShapeButton(ToolboxPanel & tbp, u32 shape, MasterIconZSheetSlot slot, const char * name, const char * doc)
        : AbstractButton()
        , m_toolbox(tbp)
        , m_shape((ToolShape) shape)
      {
        if (shape >= SHAPE_COUNT)
          FAIL(ILLEGAL_ARGUMENT);

        this->SetIconSlot(slot);

        MFM_API_ASSERT_NONNULL(name);
        MFM_API_ASSERT_NONNULL(doc);

        this->Panel::SetName(name);
        this->Panel::SetDoc(doc);

        this->Panel::SetBackground(Drawing::GREY40);
        this->Panel::SetBorder(Drawing::GREY40);
      }

      ToolShape GetShape()
      {
        return m_shape;
      }

      ToolboxPanel<GC>& GetToolbox()
      {
        return m_toolbox;
      }

      /**
       * Tell this button it is or isn't the selected shape
       *
       * @param selected If \c true, this ShapeButton will
       *                  appear active. If not, it will appear
       *                  disabled.
       */
      void SetSelected(bool selected)
      {
        // DO NOT REWRITE THIS 'IF' AS A QUESTION-COLON; IT DOES NOT
        // COMPILE ON 12.04.
        if (selected)
        {
          SetEnabledBg(Drawing::GREY40);
        }
        else
        {
          SetEnabledBg(Drawing::GREY80);
        }
      }

      /**
       * This hook is invoked when a user clicks on this button,
       * therefore setting the active tool to the shape of this
       * ShapeButton.
       *
       * @param button The button on the mouse that was pressed.
       */
      virtual void OnClick(u8 button)
      {
        this->GetToolbox().SelectShapeButton(*this);
      }
    };

    struct ElementButton : public AbstractButton
    {
      virtual s32 GetSection() { return HELP_SECTION_EDITING; }
      virtual const char * GetDoc() { return "Click left/right: select element as primary/secondary"; }
      virtual bool GetKey(u32& keysym, u32& mods) { return false; }
      virtual bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

      virtual bool PostDragHandle(MouseMotionEvent& event)
      {
        CheckHover(event);
        return AbstractButton::PostDragHandle(event);
      }

      virtual bool Handle(MouseMotionEvent& mme)
      {
        CheckHover(mme);
        return AbstractButton::Handle(mme);
      }

      bool CheckHover(MouseMotionEvent& event)
      {
        if (m_parent)
        { 
          SPoint onParent = event.GetAt();
          onParent -= m_parent->GetAbsoluteLocation();
          if (m_element) 
          {
            m_parent->SetElementLabel(m_element->GetName(), onParent);
            return true;
          }
          m_parent->SetElementLabel(0, onParent);
        }
        return false;
      }

      ElementButton() :
        AbstractButton()
      {
        this->Panel::SetBorder(Drawing::GREY70);

        this->Panel::SetBackground(Drawing::GREY80);

        this->Panel::SetForeground(Drawing::GREY70);

        this->Panel::SetBorder(Drawing::GREY60);

        AbstractButton::SetEnabled(false);

        this->MovablePanel::SetMovable(false);

        this->MovablePanel::SetResizable(false);
      }

      void SetElement(Element<EC>* element)
      {
        m_element = element;

        AbstractButton::SetEnabled(!!m_element);

        if (m_element)
        {
          OString16 name;
          name.Printf("EltBtn_%s",element->GetAtomicSymbol());
          this->SetName(name.GetZString());
        }
      }

      void SetParent(ToolboxPanel<GC>* parent)
      {
        m_parent = parent;
      }

      virtual void PaintComponentNonClick(Drawing& d)
      {
        const u32 SIZE = m_parent->GetElementRenderSize();
        if(m_element)
        {
          d.SetForeground(m_element->GetElementColor());
          d.FillRect(0, 0, SIZE, SIZE);
          d.SetFont(m_parent->GetElementRenderFont());
          d.SetBackground(Drawing::BLACK);
          d.SetForeground(Drawing::WHITE);
          d.BlitBackedTextCentered(m_element->GetAtomicSymbol(),
                                   SPoint(0, 0),
                                   UPoint(SIZE, SIZE));
        }
        else
        {
          d.SetForeground(this->GetForeground());
          d.FillRect(0, 0, SIZE, SIZE);
        }
      }

      Element<EC>* GetElement()
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
      Element<EC>* m_element;

      ToolboxPanel<GC>* m_parent;
    };

    ToolButton * m_selectedToolButton;

    void SelectToolButton(ToolButton & newToolButton)
    {
      if (m_selectedToolButton)
        m_selectedToolButton->SetSelected(false);

      m_selectedToolButton = &newToolButton;
      m_selectedToolButton->SetSelected(true);

      GridToolShapeUpdater<GC> & gt = m_selectedToolButton->GetGridTool();

      m_roundShapeButton.SetSelected(false);
      m_diamondShapeButton.SetSelected(false);
      m_squareShapeButton.SetSelected(false);

      if (gt.GetMaxVariableRadius())  // If it has a radius, it supports shape
      {
        switch (gt.GetToolShape()) {
        case ROUND_SHAPE:  SelectShapeButton(m_roundShapeButton); break;
        case DIAMOND_SHAPE:  SelectShapeButton(m_diamondShapeButton); break;
        case SQUARE_SHAPE:  SelectShapeButton(m_squareShapeButton); break;
        default: FAIL(ILLEGAL_STATE);
        }
      }
      else
      {
        m_selectedShapeButton = 0;
      }
    }

    ShapeButton * m_selectedShapeButton;

    void SelectShapeButton(ShapeButton & newShapeButton)
    {
      if (m_selectedShapeButton)
        m_selectedShapeButton->SetSelected(false);

      m_selectedShapeButton = &newShapeButton;
      m_selectedShapeButton->SetSelected(true);

      if(m_selectedToolButton)
      {
        GridToolShapeUpdater<GC> & gt = m_selectedToolButton->GetGridTool();

        if (gt.GetMaxVariableRadius())  // If it has a radius, it supports shape
          gt.SetToolShape(m_selectedShapeButton->GetShape());
      }
    }

    ToolButton m_toolButtons[MAX_GRIDTOOL_BUTTONS];
    u32 m_toolButtonsInUse;

    SiteEditorCheckbox m_siteEditCheckbox;

    ShapeButton m_roundShapeButton;
    ShapeButton m_diamondShapeButton;
    ShapeButton m_squareShapeButton;

    Element<EC>* m_primaryElement;

    Element<EC>* m_secondaryElement;

    Element<EC>* m_heldElements[ELEMENT_BOX_SIZE];

    ElementButton m_elementButtons[ELEMENT_BOX_SIZE];

    u32 m_heldElementCount;

    ParameterController<EC> *(m_controllers[TOOLBOX_MAX_CONTROLLERS]);
    u32 m_controllerCount;

    OurSlider m_sliders[TOOLBOX_MAX_SLIDERS];
    u32 m_sliderCount;

    OurParameterControllerBool m_checkboxes[TOOLBOX_MAX_CHECKBOXES];
    u32 m_checkboxCount;

    NeighborSelectPanel<EC,R> m_neighborhoods[TOOLBOX_MAX_NEIGHBORHOODS];
    u32 m_neighborhoodCount;

    void AddController(ParameterController<EC> * spc)
    {
      MFM_API_ASSERT_NONNULL(spc);

      if (m_controllerCount >= TOOLBOX_MAX_CONTROLLERS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_controllerCount++;

      m_controllers[j] = spc;
      m_controllers[j]->SetBigText(m_bigText);
      LOG.Debug("Initting %s controllers", m_bigText ? "big" : "normal");
    }

    void AddSliderController(ElementParameter<EC> * sp)
    {
      // Sliders for S32 parameters
      // Mon Jul  6 04:43:15 2015  (trying to squeeze U32 in here too, at least temporarily)
      MFM_API_ASSERT_NONNULL(sp);

      if (m_sliderCount >= TOOLBOX_MAX_SLIDERS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_sliderCount++;

      OurSlider& s = m_sliders[j];
      s.SetParameter(sp);

      AddController(&s);
    }

    void AddCheckboxController(ElementParameter<EC> * bp)
    {
      // Checkboxes for Bool parameters
      MFM_API_ASSERT_NONNULL(bp);

      if (m_checkboxCount >= TOOLBOX_MAX_CHECKBOXES)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_checkboxCount++;

      OurParameterControllerBool & cb = m_checkboxes[j];
      cb.SetParameter(bp); // FAIL unless castable to PValueTypeBool

      AddController(&cb);
    }

    void AddNeighborhoodController(ElementParameter<EC> * np)
    {
      MFM_API_ASSERT_NONNULL(np);

      if (m_neighborhoodCount >= TOOLBOX_MAX_NEIGHBORHOODS)
      {
        FAIL(OUT_OF_RESOURCES);
      }

      u32 j = m_neighborhoodCount++;

      NeighborSelectPanel<EC,R>& nb = m_neighborhoods[j];

      nb.SetParameter(np);
      nb.SetText(np->GetName());

      AddController(&nb);
    }

   public:

    virtual s32 GetSection() { return HELP_SECTION_EDITING; }
    virtual const char * GetDoc() { return "A collection of grid editing tools"; }
    virtual bool GetKey(u32& keysym, u32& mods) { return false; }
    virtual bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

    bool IsSiteEdit()
    {
      return m_siteEdit;
    }

    void SetSiteEdit(bool edit)
    {
      m_siteEdit = edit;
    }


    AbstractButton & RegisterGridTool(GridToolShapeUpdater<GC> & gt)
    {
      if (m_toolButtonsInUse >= MAX_GRIDTOOL_BUTTONS)
        FAIL(OUT_OF_ROOM);
      ToolButton & tb = m_toolButtons[m_toolButtonsInUse++];
      tb.SetGridTool(gt);
      tb.Panel::SetDoc(gt.GetDoc());
      if (m_toolButtonsInUse <= 9)
      {
        tb.SetKeysym(m_toolButtonsInUse + '0', 0);
      }
      return tb;
    }

    void SetElementLabel(const char * label, const SPoint at) 
    {
      m_hoverElementLabel = label;
      m_hoverElementLabelAt = at;
    }

    ToolboxPanel()
      : m_hoverElementLabel(0)
      , m_bigText(false)
      , m_siteEdit(false)
      , m_selectedToolButton(0)
      , m_selectedShapeButton(0)
      , m_toolButtonsInUse(0)
      , m_siteEditCheckbox(*this)
      , m_roundShapeButton(*this, ROUND_SHAPE, ZSLOT_TOOLSHAPE_ROUND, "Round", "Use a round-shaped brush")
      , m_diamondShapeButton(*this, DIAMOND_SHAPE, ZSLOT_TOOLSHAPE_DIAMOND, "Diamond", "Use a diamond-shaped brush")
      , m_squareShapeButton(*this, SQUARE_SHAPE, ZSLOT_TOOLSHAPE_SQUARE, "Square", "Use a square-shaped brush")
      , m_primaryElement(&Element_Empty<EC>::THE_INSTANCE)
      , m_secondaryElement(&Element_Empty<EC>::THE_INSTANCE)
      , m_heldElementCount(0)
      , m_controllerCount(0)
      , m_sliderCount(0)
      , m_checkboxCount(0)
    {
      SetName("ToolboxPanel");

      for(u32 i = 0; i < TOOLBOX_MAX_CONTROLLERS; i++)
      {
        m_controllers[i] = 0;
      }

      for(u32 i = 0; i < MAX_GRIDTOOL_BUTTONS; i++)
      {
        m_toolButtons[i].SetToolbox(*this);
      }

      for(u32 i = 0; i < ELEMENT_BOX_SIZE; i++)
      {
        m_heldElements[i] = NULL;
      }

      for(u32 i = 0; i < TOOLBOX_MAX_SLIDERS; i++)
      {
        OString16 str;
        str.Printf("Slider%D",i);
        m_sliders[i].SetName(str.GetZString());
      }

      for(u32 i = 0; i < TOOLBOX_MAX_CHECKBOXES; i++)
      {
        OString16 str;
        str.Printf("Checkbox%D",i);
        m_checkboxes[i].SetName(str.GetZString());
      }

      for(u32 i = 0; i < TOOLBOX_MAX_NEIGHBORHOODS; i++)
      {
        OString16 str;
        str.Printf("Neighborhood%D",i);
        m_neighborhoods[i].SetName(str.GetZString());
      }


    }

    /**
     * Sets whether or not this ToolboxPanel should render its
     * components at a large or normal size.
     *
     * @param value If \c true , this ToolboxPanel will render its
     *              child components at an increased size. Else, will
     *              render its children at the normal size.
     */
    void SetBigText(bool value)
    {
      m_bigText = value;

      for(u32 i = 0; i < TOOLBOX_MAX_CONTROLLERS; i++)
      {
        if(m_controllers[i])
        {
          m_controllers[i]->SetBigText(value);
        }
      }
    }

    void AddElementParameters()
    {

      /* Set up the new controllers, if any */
      ElementParameters<EC> & parms = m_primaryElement->GetElementParameters();
      u32 totalParms = parms.GetParameterCount();

      m_sliderCount = 0;
      m_checkboxCount = 0;
      m_neighborhoodCount = 0;
      for(u32 i = 0; i < totalParms; i++)
      {
        ElementParameter<EC> * parm = parms.GetParameter(i);
        switch (parm->GetType())
        {
        case VD::UNARY:
        case VD::U32:
        case VD::S32:
          AddSliderController(parm);
          break;
        case VD::BOOL:
          AddCheckboxController(parm);
          break;
        case VD::BITS:
          {
            ElementParameterNeighborhood<EC,SITES>* epn =
              dynamic_cast<ElementParameterNeighborhood<EC,SITES>*>(parm);
            if (epn)
            {
              AddNeighborhoodController(epn);
            }
            break;
          }
          // ELSE FALL THROUGH
        default:
          FAIL(ILLEGAL_STATE);
        }
      }

      /* Put in the new controllers */
      SPoint rpt(16, 6 + GetElementRenderSize() * TOTAL_ROWS);
      for(u32 i = 0; i < m_controllerCount; i++)
      {
        ParameterController<EC> * c = m_controllers[i];
        c->Init();
        c->SetRenderPoint(rpt);
        c->SetBigText(m_bigText);
        Panel::Insert(c, NULL);
        UPoint desired = c->GetDesiredSize();
        rpt.SetY(rpt.GetY() + desired.GetY());
      }

      Panel::SetDimensions(TOOLBOX_WIDTH, rpt.GetY() + BORDER_PADDING);
      Panel::SetDesiredSize(TOOLBOX_WIDTH, rpt.GetY() + BORDER_PADDING);
    }

    void RebuildControllers()
    {
      /* Remove any old controllers */
      for(u32 i = 0; i < m_controllerCount; i++)
      {
        Panel::Remove(m_controllers[i]);
      }
      m_controllerCount = 0;

      AddElementParameters();

      //debug: this->Print(STDOUT);

      // Tell ourselfs to resize (our cheeso repacking)
      HandleResize(this->GetParent()->GetDimensions());

    }


    void AddButtons()
    {
      // Clear all then re-add
      Panel * p;
      while ((p = this->GetTop())) this->Remove(p);

      for(u32 i = 0; i < m_toolButtonsInUse; i++)
      {
        OString16 name;
        name.Printf("ToolButton%D",i);
        m_toolButtons[i].GetGridTool(); // Blow assertion if we don't have a tool by now
        m_toolButtons[i].Init();
        m_toolButtons[i].SetSelected(false);
        m_toolButtons[i].SetName(name.GetZString());
        m_toolButtons[i].SetToolbox(*this);
        m_toolButtons[i].Panel::SetDimensions(GetElementRenderSize(),
                                              GetElementRenderSize());
        m_toolButtons[i].Panel::SetRenderPoint(SPoint(16 + i * GetElementRenderSize(), 3));
        Panel::Insert(m_toolButtons + i, NULL);
      }

      u32 imgSize = 22;
      u32 btnSize = 26;
      const SPoint shapeSize(btnSize,btnSize);
      u32 iconIndent = (btnSize - imgSize) / 2;
      const SPoint iconAt(iconIndent, iconIndent);

      m_roundShapeButton.Panel::SetRenderPoint(SPoint(16 + (m_toolButtonsInUse + 1) * GetElementRenderSize(), 3));
      m_diamondShapeButton.Panel::SetRenderPoint(SPoint(16 + (m_toolButtonsInUse + 2) * GetElementRenderSize(), 3));
      m_squareShapeButton.Panel::SetRenderPoint(SPoint(16 + (m_toolButtonsInUse + 3) * GetElementRenderSize(), 3));

      m_roundShapeButton.Panel::SetDimensions(btnSize,btnSize);
      m_diamondShapeButton.Panel::SetDimensions(btnSize,btnSize);
      m_squareShapeButton.Panel::SetDimensions(btnSize,btnSize);

      m_roundShapeButton.Label::SetIconPosition(iconAt);
      m_diamondShapeButton.Label::SetIconPosition(iconAt);
      m_squareShapeButton.Label::SetIconPosition(iconAt);

      Panel::Insert(&m_roundShapeButton, NULL);
      Panel::Insert(&m_diamondShapeButton, NULL);
      Panel::Insert(&m_squareShapeButton, NULL);

      m_siteEditCheckbox.Panel::SetRenderPoint(SPoint(0,0)); // put anywhere, let startup-file sort it out
      m_siteEditCheckbox.Panel::SetDimensions(200,100); // ditto
      Panel::Insert(&m_siteEditCheckbox, NULL);

      for(u32 i = 0; i < ELEMENT_BOX_SIZE; i++)
      {
        u32 x, y;

        x = 3 + GetElementRenderSize() * (i % ELEMENTS_PER_ROW);
        y = 3 + GetElementRenderSize() * (3 + (i / ELEMENTS_PER_ROW));

        /* These will render correctly because ElementButtons render
           grey when SetElement is fed NULL. */
        OString16 name;
        name.Printf("Element%D",i);
        m_elementButtons[i].SetElement(m_heldElements[i]);
        m_elementButtons[i].SetName(name.GetZString());
        m_elementButtons[i].SetParent(this);
        m_elementButtons[i].Panel::SetDimensions(GetElementRenderSize(),
                                                 GetElementRenderSize());
        m_elementButtons[i].Panel::SetRenderPoint(SPoint(x, y));

        Panel::Insert(m_elementButtons + i, NULL);
      }

      SetPrimaryElement(m_heldElements[0]);
      SetSecondaryElement(m_heldElements[1]);

      Panel::SetDimensions(6 + GetElementRenderSize() * ELEMENTS_PER_ROW,
                           6 + GetElementRenderSize() * TOTAL_ROWS);
      Panel::SetDesiredSize(6 + GetElementRenderSize() * ELEMENTS_PER_ROW,
                            6 + GetElementRenderSize() * TOTAL_ROWS);

    }

    void SetPrimaryElement(Element<EC>* element)
    {
      if (element)
        m_primaryElement = element;
      else
        m_primaryElement = &Element_Empty<EC>::THE_INSTANCE;
    }

    Element<EC>* GetPrimaryElement()
    {
      return m_primaryElement;
    }

    void SetSecondaryElement(Element<EC>* element)
    {
      if (element)
        m_secondaryElement = element;
      else
        m_secondaryElement = &Element_Empty<EC>::THE_INSTANCE;
    }

    Element<EC>* GetSecondaryElement()
    { return m_secondaryElement; }

    GridTool<GC> * GetCurrentTool()
    {
      if (!m_selectedToolButton) return 0;
      return &m_selectedToolButton->GetGridTool();
    }

    void RegisterElement(Element<EC>* element)
    {
      if(m_heldElementCount >= ELEMENT_BOX_SIZE)
      {
        FAIL(OUT_OF_ROOM);
      }
      m_heldElements[m_heldElementCount++] = element;
    }

    virtual bool PostDragHandle(MouseButtonEvent& mbe)
    {
      SDL_MouseButtonEvent & event = mbe.m_event.button;

      /* Only fire on wheel 'press' to avoid double-counting */
      if(event.type != SDL_MOUSEBUTTONDOWN)
      {
        return true;
      }

      if(m_selectedToolButton)
      {
        GridToolShapeUpdater<GC> & gt = m_selectedToolButton->GetGridTool();
        u32 radius = gt.GetRadius();

        if (radius > 0 &&
            (event.button == SDL_BUTTON_WHEELUP ||
             event.button == SDL_BUTTON_WHEELDOWN))
          gt.Press(mbe);
      }

      return true;
    }

    virtual bool PostDragHandle(MouseMotionEvent& mme)
    {
      SPoint s;
      SetElementLabel(0, s);

      /* Try to keep the grid from taking this event too */
      return true;
    }

    virtual void PaintComponent(Drawing& d)
    {
      const u32 ELEMENT_START_X = 20;
      const u32 ELEMENT_START_Y = m_bigText ? 64 : 40;
      const u32 ELEMENT_TOOL_SIZE = GetElementRenderSize() + 8;

      d.SetForeground(this->Panel::GetBackground());
      d.FillRect(0, 0, this->Panel::GetWidth(), this->Panel::GetHeight());

      d.SetFont(GetElementRenderFont());

      if (m_primaryElement)
      {
        d.SetForeground(m_primaryElement->GetStaticColor());
        d.FillCircle(ELEMENT_START_X, ELEMENT_START_Y, ELEMENT_TOOL_SIZE,
                     ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE / 2);

        d.SetBackground(Drawing::BLACK);
        d.SetForeground(Drawing::WHITE);
        d.BlitBackedTextCentered(m_primaryElement->GetAtomicSymbol(),
                                 SPoint(ELEMENT_START_X, ELEMENT_START_Y),
                                 UPoint(ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE));
      }
      if (m_secondaryElement)
      {
        const u32 SECONDARY_X_START = ELEMENT_START_X + ELEMENT_TOOL_SIZE;
        d.SetForeground(m_secondaryElement->GetStaticColor());
        d.FillCircle(SECONDARY_X_START, ELEMENT_START_Y, ELEMENT_TOOL_SIZE,
                     ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE / 2);

        d.SetBackground(Drawing::BLACK);
        d.SetForeground(Drawing::WHITE);
        d.BlitBackedTextCentered(m_secondaryElement->GetAtomicSymbol(),
                                 SPoint(SECONDARY_X_START, ELEMENT_START_Y),
                                 UPoint(ELEMENT_TOOL_SIZE, ELEMENT_TOOL_SIZE));
      }

      if(m_selectedToolButton)
      {
        GridToolShapeUpdater<GC> & gt = m_selectedToolButton->GetGridTool();
        u32 radius = gt.GetRadius();

        if (radius > 0)
        {
          d.SetBackground(Drawing::BLACK);
          d.SetForeground(Drawing::WHITE);

          char brushSizeArray[64];

          snprintf(brushSizeArray, 64, "%2d", radius);

          const SPoint brushPos = m_toolButtons[m_toolButtonsInUse - 1].Panel::GetRenderPoint();
          SPoint pos(brushPos.GetX() + GetElementRenderSize(), brushPos.GetY());
          d.BlitBackedText(brushSizeArray, pos, UPoint(128, 128));
        }
      }
    }

    virtual void PaintFloat(Drawing& d)
    {

      if (m_hoverElementLabel)
      {
        d.SetFont(FONT_ASSET_BUTTON_BIG);

        UPoint tsize = MakeUnsigned(d.GetTextSize(m_hoverElementLabel)); // praying for no font failures urgh
        UPoint offset(8,16);
        UPoint border(4,4);

        s32 xpos, ypos;
        u32 wid, hei;
        xpos = m_hoverElementLabelAt.GetX() - border.GetX()/2 + offset.GetX();
        ypos = m_hoverElementLabelAt.GetY() - border.GetY()/2 + offset.GetY();
        wid = tsize.GetX() + border.GetX();
        hei = tsize.GetY() + border.GetY();

        // Get as much as possible in the window (favoring a prefix if
        // need be)

        Rect wdw;
        d.GetWindow(wdw);

        if (xpos < 0 || wid > wdw.GetWidth()) xpos = 0;
        else if (xpos + wid > wdw.GetWidth()) xpos = wdw.GetWidth() - wid;

        if (ypos < 0 || hei > wdw.GetHeight()) ypos = 0;
        else if (ypos + hei > wdw.GetHeight()) ypos = wdw.GetHeight() - hei;

        d.FillRect(xpos, ypos, wid, hei, Drawing::YELLOW);
        d.SetForeground(Drawing::BLACK);
        d.BlitText(m_hoverElementLabel, SPoint(xpos, ypos), tsize);
      }

    }

    void SaveDetails(ByteSink & sink) const
    {
      Super::SaveDetails(sink);

      s32 selIdx = -1;
      for(u32 i = 0; i < m_toolButtonsInUse; i++)
      {
        if (m_selectedToolButton == &m_toolButtons[i])
          selIdx = i;
        m_toolButtons[i].GetGridTool().SaveDetails(sink);
      }

      sink.Printf(" PP(stbi=%d)\n", selIdx);
      sink.Printf(" PP(site=%d)\n", m_siteEdit);
    }

    bool LoadDetails(const char * key, LineCountingByteSource & source)
    {
      if (Super::LoadDetails(key, source)) return true;

      for(u32 i = 0; i < m_toolButtonsInUse; i++)
      {
        if (m_toolButtons[i].GetGridTool().LoadDetails(key, source))
          return true;
      }

      if (!strcmp(key,"stbi"))
      {
        s32 selIdx;
        if (1 != source.Scanf("%?d", sizeof selIdx, &selIdx))
          return false;
        for(u32 i = 0; i < m_toolButtonsInUse; i++)
        {
          m_toolButtons[i].SetSelected(false);
        }
        if (selIdx >= 0 && selIdx < (s32) m_toolButtonsInUse)
          m_toolButtons[selIdx].OnClick(SDL_BUTTON_LEFT);
        return true;
      }

      if (!strcmp(key,"site")) return 1 == source.Scanf("%?d", sizeof m_siteEdit, &m_siteEdit);

      return false;
    }

  };
}

#endif /* TOOLBOXPANEL_H */
