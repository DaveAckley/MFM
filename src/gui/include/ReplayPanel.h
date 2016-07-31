/*                                              -*- mode:C++ -*-
  ReplayPanel.h A Panel for accessing event history
  Copyright (C) 2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file ReplayPanel.h A Panel for accessing event history
  \author Dave Ackley
  \date (C) 2016 All rights reserved.
  \lgpl
*/
#ifndef REPLAYPANEL_H
#define REPLAYPANEL_H

#include "AbstractButton.h"
#include "AbstractCheckbox.h"
#include "Slider.h"
#include "MovablePanel.h"

namespace MFM
{
  template <class GC> class Grid;    // FORWARD
  template <class GC> class GridPanel;    // FORWARD

  /** Panel providing an interface to the event history recording
      system
   */
  template<class GC>
  class ReplayPanel : public MovablePanel
  {
  private:
    typedef MovablePanel Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    typedef GridPanel<GC> OurGridPanel;
    typedef Grid<GC> OurGrid;

    typedef EventHistoryBuffer<EC> OurEventHistoryBuffer;
    typedef Tile<EC> OurTile;

    typedef Site<AC> OurSite;

    enum {
      R = EC::EVENT_WINDOW_RADIUS,
      SITES = EVENT_WINDOW_SITES(R)
    };

    class TransportButton : public AbstractButton
    {
    protected:
      ReplayPanel<GC> & m_replayPanel;
      s32 m_keysym;
      u32 m_mods;
      s32 m_delta;

    public:
      TransportButton(const char * name, 
                      const char * doc,
                      s32 keysym,
                      u32 mods,
                      u32 iconSlot,
                      s32 actionDelta,
                      ReplayPanel<GC> & rp)
        : AbstractButton(name)
        , m_replayPanel(rp)
        , m_keysym(keysym)
        , m_mods(mods)
        , m_delta(actionDelta)
      {
        Panel::SetDoc(doc);
        Panel::SetFont(FONT_ASSET_BUTTON_SMALL);

        this->SetEnabledBg(Drawing::BLACK);
        this->SetEnabledFg(Drawing::WHITE);

        IconAsset & ia = this->GetIconAsset();
        ia.SetIconSlot(iconSlot);

        this->SetDimensions(SPoint(25, 25));
        this->SetDesiredSize(25, 25);
        this->SetRenderPoint(SPoint(25 * iconSlot, 0));

        PanelNameString & pname = this->GetWritableName();
        pname.Reset();
        pname.Printf("TransportButton%D",iconSlot);

        rp.Insert(this, 0);
      }

      virtual void UpdateEnabling() ;

      virtual void OnClick(u8 button) ;

      virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mods)
      {
        if (m_keysym < 0)
          return false;
        keysym = m_keysym;
        mods = m_mods;
        return true;
      }

      virtual s32 GetSection() { return HELP_SECTION_EDITING; }
      virtual const char * GetDoc() { return Panel::GetDoc(); }
      virtual bool GetKey(u32& keysym, u32& mods)
      {
        return this->GetKeyboardAccelerator(keysym, mods);
      }

      virtual bool ExecuteFunction(u32 keysym, u32 mods) {
        if (m_keysym >= 0 && keysym == (u32) m_keysym && mods == mods)
        {
          OnClick(SDL_BUTTON_LEFT);
          return true;
        }
        return false;
      }
    };

    class TransportButtonWoahBack : public TransportButton
    {
      const bool m_direction;

    public:
      TransportButtonWoahBack(const char * name, 
                              const char * doc,
                              s32 keysym,
                              u32 mods,
                              u32 iconSlot,
                              bool forward,
                              ReplayPanel<GC> & rp)
        : TransportButton(name,doc,keysym,mods,iconSlot,forward? 1: -1, rp)
        , m_direction(forward)
      { }

      void OnClick(u8 button)
      {
        ReplayPanel<GC> & rp = this->m_replayPanel;
        if ((rp.m_lastTransportDelta >= 0) == m_direction)
        {
          // Accelerate linearly
          rp.m_lastTransportDelta += (m_direction ? 5 : -5);
        }
        else
        {
          // Woah back!  Decelerate exponentially
          rp.m_lastTransportDelta = -rp.m_lastTransportDelta / 2;
        }
        if (rp.m_lastTransportDelta == 0)
          rp.m_lastTransportDelta = (m_direction ? 1 : -1);
        this->m_delta = rp.m_lastTransportDelta;
        TransportButton::OnClick(button);
      }
    };

    OurGridPanel & m_gridPanel;
    TransportButton m_transportHome;
    TransportButtonWoahBack m_transportRew;
    TransportButton m_transportBack;
    TransportButton m_transportStep;
    TransportButtonWoahBack m_transportFF;
    TransportButton m_transportEnd;
    s32 m_lastTransportDelta; // For woah-back

    /**
     * A class representing a Button for selecting a particular
     * GridTool.
     */
    class ToolButton : public AbstractButton
    {
    private:
      ReplayPanel<GC>* m_toolbox;
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
        , m_toolbox(0)
        , m_keysym(-1)
        , m_mods(0)
      {
        this->Panel::SetBackground(Drawing::GREY40);
        this->Panel::SetBorder(Drawing::GREY40);
      }


      void SetKeysym(s32 key, u32 mods)
      {
        m_keysym = key;
        m_mods = mods;
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

   public:

    virtual s32 GetSection() { return HELP_SECTION_EDITING; }
    virtual const char * GetDoc() { return "A collection of grid editing tools"; }
    virtual bool GetKey(u32& keysym, u32& mods) { return false; }
    virtual bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

    GridPanel<GC> & GetGridPanel() { return m_gridPanel; }

    ReplayPanel(GridPanel<GC> & gp)
      : m_gridPanel(gp)
      , m_transportHome("Begin",
                        "Go to oldest event in selected tile(s)",
                        SDLK_HOME, 0, 
                        ZSLOT_TRANSPORT_OLDEST, S32_MIN,
                        *this)
      , m_transportRew("Rewind",
                       "Go back more and more events in selected tile(s)",
                       SDLK_LEFT, KMOD_SHIFT, 
                       ZSLOT_TRANSPORT_REWIND, false,
                       *this)
      , m_transportBack("Back",
                       "Go back one event in selected tile(s)",
                       SDLK_LEFT, 0, 
                        ZSLOT_TRANSPORT_BACK, -1,
                       *this)
      , m_transportStep("Step",
                       "Go forward one event in selected tile(s)",
                       SDLK_RIGHT, 0, 
                       ZSLOT_TRANSPORT_STEP, 1,
                       *this)
      , m_transportFF("Fast forward",
                      "Go forward more and more events in selected tile(s)",
                      SDLK_RIGHT, KMOD_SHIFT, 
                      ZSLOT_TRANSPORT_FF, true,
                      *this)
      , m_transportEnd("End",
                       "Go forward to newest event in selected tile(s)",
                       SDLK_END, 0, 
                       ZSLOT_TRANSPORT_NEWEST, S32_MAX,
                       *this)
      , m_lastTransportDelta(0)
      
    {
      this->SetDimensions(130, 25);
      this->SetDesiredSize(130, 25);
      this->SetFont(FONT_ASSET_HELPPANEL_SMALL);
      this->SetName("ReplayPanel");

    }

    // Is this how we say no border?
    virtual void PaintBorder(Drawing& d)
    {
      /* empty */
    }

    virtual void PaintFloat(Drawing& d)
    {

#if 0
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
#endif

    }

    void SaveDetails(ByteSink & sink) const
    {
      Super::SaveDetails(sink);

#if 0
      s32 selIdx = -1;
      for(u32 i = 0; i < m_toolButtonsInUse; i++)
      {
        if (m_selectedToolButton == &m_toolButtons[i])
          selIdx = i;
        m_toolButtons[i].GetGridTool().SaveDetails(sink);
      }

      sink.Printf(" PP(stbi=%d)\n", selIdx);
      sink.Printf(" PP(site=%d)\n", m_siteEdit);
#endif
    }

    bool LoadDetails(const char * key, LineCountingByteSource & source)
    {
      if (Super::LoadDetails(key, source)) return true;

#if 0
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
#endif
      return false;
    }

  };

}

#include "ReplayPanel.tcc"

#endif /* REPLAYPANEL_H */
