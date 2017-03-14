/*                                              -*- mode:C++ -*-
  AtomViewPanel.h Panel displaying details of selected Atom
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
  \file AtomViewPanel.h Panel displaying details of selected Atom
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ATOMVIEWPANEL_H
#define ATOMVIEWPANEL_H

#include "CloseWindowButton.h"
#include "MovablePanel.h"
#include "Grid.h"
#include "TreeViewPanel.h"

namespace MFM
{
  template <class GC> class GridPanel; // FORWARD

  template <class GC>
  class AtomViewPanel : public MovablePanel
  {
   private:
    typedef MovablePanel Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    SPoint m_gridCoord;
    bool m_inBase;

    Grid<GC>* m_grid;
    GridPanel<GC>* m_gridPanel;

    struct ClearAtomCoordButton : public CloseWindowButton
    {
      typedef CloseWindowButton Super;

      const char * GetDoc() { return "Close atom view panel"; }
      virtual bool GetKey(u32& keysym, u32& mods) { return false; }
      bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

      ClearAtomCoordButton(AtomViewPanel<GC> & avp)
        : Super(&avp)
        , m_avp(avp)
      { }

      AtomViewPanel<GC> & m_avp;

      virtual void OnClick(u8 button)
      {
        m_avp.ClearAtomCoord();
      }
    };

    ClearAtomCoordButton m_closeWindowButton;
    TreeViewPanel m_treeViewPanel;

    static const u32 ATOM_DRAW_SIZE = 40;

   public:
    AtomViewPanel()
      : MovablePanel(300, 100)
      , m_gridCoord(-1,-1)
      , m_inBase(false)
      , m_grid(NULL)
      , m_gridPanel(NULL)
      , m_closeWindowButton(*this) // Inserts itself into us
      , m_treeViewPanel()
    {
      SetName("AtomViewPanel");
      SetBackground(Drawing::GREY20);
      SetForeground(Drawing::GREY80);

      // Mon Jun  1 03:33:42 2015 NOT READY FOR PRIME TIME
      // this->Panel::Insert(&m_treeViewPanel, NULL);
    }

    bool HasGridCoord() const
    {
      return m_gridCoord.GetX() >= 0 && m_gridCoord.GetY() >= 0;
    }

    const SPoint & GetGridCoord() const
    {
      return m_gridCoord;
    }

    void Init()
    {
      Panel::SetDesiredSize(400, 200);
      m_closeWindowButton.Init();
      m_treeViewPanel.Init();
    }

    virtual void PaintUpdateVisibility(Drawing& d)
    {
      this->SetVisible(HasGridCoord());
    }

    void PaintDisplayAtomicControllers(Drawing & d, T& atom,const Element<EC>* elt)
    {
      // XXX DESIGN ME
      // XXX WRITE ME
      // XXX MAKE ME WORK

      const AtomicParameters<EC> & parms = elt->GetAtomicParameters();

      const AtomicParameter<EC> * p;
      for (p = parms.GetFirstParameter(); p; p = p->GetNextParameter())
      {
        switch (p->GetType())
        {
        case VD::U32:
          {
            u32 val = 0;
            bool got = p->LoadU32(atom, val);
            LOG.Debug("u32 %s = %d (%d)", p->GetName(), (s32) val, got);
          }
          break;
        default:
          LOG.Debug("u32 %s unknown type %d", p->GetName(), p->GetType());
        }
      }
    }

    void DrawElementLogo(Drawing & d, FontAsset font, s32 xstart, s32 ystart, u32 drawSize, u32 color, const char * symbol)
    {
      d.SetForeground(Panel::GetForeground());
      d.FillCircle(xstart-1, ystart-1, drawSize + 2, drawSize + 2, (drawSize >> 1) + 1);
      d.SetForeground(color);
      d.FillCircle(xstart, ystart, drawSize, drawSize, drawSize >> 1);
      d.SetFont(font);
      d.SetForeground(Drawing::WHITE);
      d.SetBackground(Drawing::BLACK);

      /* Center on the draw_size x draw_size box */
      d.BlitBackedTextCentered(symbol,
                               SPoint(xstart, ystart), UPoint(drawSize, drawSize));

    }

    void PrintLine(Drawing & d, const u32 baseX, u32 & currentY, const u32 lineHeight, const char * zstr)
    {
      d.BlitBackedText(zstr, SPoint(baseX, currentY),
                       MakeUnsigned(d.GetTextSize(zstr)));
      currentY += lineHeight;
    }

    /**
       Highlight our border if we're the selected one
     */
    virtual void PaintBorder(Drawing & d)
    {
      u32 oldBd = GetBorder();
      if (GetGridPanel().IsSelectedAtomViewPanel(*this))
        SetBorder(Drawing::YELLOW);

      this->Super::PaintBorder(d);
      SetBorder(oldBd);
    }

    virtual void PaintComponent(Drawing& d)
    {
      this->Super::PaintComponent(d);

      //      d.DrawLineDitColor(10000, 20000, 0, 55000, Drawing::BLUE);

      if(!HasGridCoord())
      {
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        const char* message = "No atom selected.";
        d.BlitText(message, SPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
        return;
      }

      Grid<GC>& grid = this->GetGrid();
      const T* catom = grid.GetAtomInSite(m_inBase, m_gridCoord);
      if (!catom)
        FAIL(INCOMPLETE_CODE); // what to do?
      T atom(*catom); // Get a non-const atom so serializer can use it sigh.

      OString512 buff;
      AtomSerializer<AC> serializer(atom);

      const u32 ATOM_X_START = 20;
      const u32 ATOM_Y_START = 25;
      u32 baseX = ATOM_X_START;
      u32 currentY = 2;

      const u32 etype = atom.GetType();
      const Element<EC>* element = m_grid->LookupElement(etype);
      {
        SPoint tileInGrid, siteInTile;
        if (!grid.MapGridToTile(m_gridCoord, tileInGrid, siteInTile))
          FAIL(ILLEGAL_STATE);

        SPointSerializer tsp(tileInGrid);
        SPointSerializer ssp(siteInTile);
        buff.Reset();
        buff.Printf("%s Layer, Site %@, Tile %@",
                    m_inBase?"Base":"Event",
                    &ssp, &tsp);
        const char * str = buff.GetZString();
        d.SetFont(FONT_ASSET_LOGGER);
        d.BlitBackedText(str, SPoint(baseX, currentY), MakeUnsigned(d.GetTextSize(str)));
      }

      currentY = ATOM_Y_START;

      if (!element)
      {
        buff.Printf("Unknown type '0x%04x' in: %@", etype, &serializer);
        const char * str = buff.GetZString();
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(str, SPoint(32, 32), MakeUnsigned(d.GetTextSize(str)));
        return;
      }

      u32 staticColor = element->GetStaticColor();
      const char * sym = element->GetAtomicSymbol();
      this->DrawElementLogo(d, FONT_ASSET_ELEMENT, baseX, currentY, ATOM_DRAW_SIZE, staticColor, sym);

      u32 curX = baseX + ATOM_DRAW_SIZE;
      const u32 DCOLOR_SIZE = ATOM_DRAW_SIZE - 8;
      const Tile<EC>& tile = grid.Get00Tile();
      const ElementTable<EC>& et = tile.GetElementTable();
      const UlamClassRegistry<EC>& ucr = tile.GetUlamClassRegistry();
      u32 dcolor[3];
      dcolor[0] = staticColor;
      dcolor[1] = element->GetDynamicColor(et, ucr, atom, 1);
      dcolor[2] = element->GetDynamicColor(et, ucr, atom, 2);
      bool show1 = false, show2 = false, bothsame = false;
      if (dcolor[1] != dcolor[0]) show1 = true;
      if (dcolor[2] != dcolor[0]) show2 = true;
      if (dcolor[2] == dcolor[1]) {
        bothsame = true;
        show2 = false;
      }
      if (show1)
      {
        const char * l = bothsame ? "" : "1";
        this->DrawElementLogo(d, FONT_ASSET_ELEMENT_SMALL, curX, currentY, DCOLOR_SIZE, dcolor[1], l);
        curX += DCOLOR_SIZE;
      }
      if (show2)
      {
        this->DrawElementLogo(d, FONT_ASSET_ELEMENT_SMALL, curX, currentY, DCOLOR_SIZE, dcolor[2], "2");
        curX += DCOLOR_SIZE;
      }

      UPoint nameSize = MakeUnsigned(d.GetTextSize(element->GetName()));
      d.BlitBackedText(element->GetName(),
                       SPoint(curX,
                              currentY + ATOM_DRAW_SIZE - nameSize.GetY()), // bottom align text
                       nameSize);

      currentY += ATOM_DRAW_SIZE;

      buff.Reset();
      element->AppendDescription(atom, buff);
      const char* zstr = buff.GetZString();

      d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
      const u32 LINE_X_START = baseX + 4;
      const u32 LINE_HEIGHT = AssetManager::GetFontLineSkip(d.GetFont());
      const u32 INDENT_AMOUNT = LINE_HEIGHT;

      this->PrintLine(d, LINE_X_START, currentY, LINE_HEIGHT, zstr);

      buff.Reset();
      buff.Printf("%@", &serializer);
      zstr = buff.GetZString();

      this->PrintLine(d, LINE_X_START, currentY, LINE_HEIGHT, zstr);

      const UlamElement<EC> * uelt = element->AsUlamElement();
      if (!uelt)
      {
        PaintDisplayAtomicControllers(d, atom, element);
        return;
      }

      const u32 printFlags =
        UlamClassPrintFlags::PRINT_MEMBER_NAMES |
        UlamClassPrintFlags::PRINT_MEMBER_VALUES |
        UlamClassPrintFlags::PRINT_RECURSE_QUARKS;

      buff.Reset();
      uelt->Print(ucr, buff, atom, printFlags, T::ATOM_FIRST_STATE_BIT);
      zstr = buff.GetZString();
      u32 indent = 0;
      u32 lineNum = 1;
      OString64 lineBuff;

      for (u8 ch = *zstr; ch; ch = *++zstr)
      {
        u32 oldIndent = indent;
        u32 oldLineNum = lineNum;
        if (ch == '(')
        {
          ++lineNum;
          ++indent;
        }
        else if (ch == ')')
        {
          --indent;
        }
        else if (ch == ',')
        {
          ++lineNum;
        }
        if (oldIndent != indent || oldLineNum != lineNum)
        {
          if (lineBuff.GetLength() > 0)
          {
            const char * line = lineBuff.GetZString();
            d.BlitBackedText(line,
                             SPoint(baseX + oldIndent * INDENT_AMOUNT, currentY + oldLineNum * LINE_HEIGHT),
                             MakeUnsigned(d.GetTextSize(line)));
            lineBuff.Reset();
          }
        } else
        {
          lineBuff.Printf("%c", ch);
        }
      }
      UPoint ds = Panel::GetDesiredSize();
      u32 dy = MAX(200u,currentY + (lineNum + 1) * LINE_HEIGHT);
      if (dy != ds.GetY())
      {
        ds.SetY(dy);
        Panel::SetDesiredSize(ds.GetX(), ds.GetY());
        Panel * parent = this->GetParent();
        if (parent)
          HandleResize(parent->GetDimensions());
      }
    }

    virtual bool Handle(KeyboardEvent& kbe) ;

    virtual bool Handle(MouseButtonEvent& mbe) ;

    virtual bool Handle(MouseMotionEvent& mme) ;

    void ClearAtomCoord()
    {
      m_gridCoord = SPoint(-1,-1);
    }

    void SetAtomCoord(SPoint gridCoord, bool inBase)
    {
      m_gridCoord = gridCoord;
      m_inBase = inBase;
    }

    void SetGrid(Grid<GC>& grid)
    {
      MFM_API_ASSERT_NULL(m_grid);
      m_grid = &grid;
    }

    Grid<GC> & GetGrid()
    {
      MFM_API_ASSERT_NONNULL(m_grid);
      return *m_grid;
    }

    void SetGridPanel(GridPanel<GC>& gridPanel)
    {
      MFM_API_ASSERT_NULL(m_gridPanel);
      m_gridPanel = &gridPanel;
    }

    GridPanel<GC> & GetGridPanel()
    {
      MFM_API_ASSERT_NONNULL(m_gridPanel);
      return *m_gridPanel;
    }

  };
}

#include "AtomViewPanel.tcc"

#endif /* ATOMVIEWPANEL_H */
