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
#include "ToolboxPanel.h"
#include "TreeViewPanel.h"

namespace MFM
{
  template <class GC>
  class AtomViewPanel : public MovablePanel
  {
   private:
    typedef MovablePanel Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    T* m_atom;

    Grid<GC>* m_grid;

    ToolboxPanel<EC>* m_toolboxPanel;

    CloseWindowButton m_closeWindowButton;
    TreeViewPanel m_treeViewPanel;

    static const u32 ATOM_DRAW_SIZE = 40;

   public:
    AtomViewPanel()
      : MovablePanel(300, 100)
      , m_atom(NULL)
      , m_grid(NULL)
      , m_toolboxPanel(NULL)
      , m_closeWindowButton(this) // Inserts itself into us
      , m_treeViewPanel()
    {
      SetBackground(Drawing::GREY20);
      SetForeground(Drawing::GREY80);

      // Mon Jun  1 03:33:42 2015 NOT READY FOR PRIME TIME
      // this->Panel::Insert(&m_treeViewPanel, NULL);
    }

    void Init()
    {
      Panel::SetDesiredSize(400, 200);
      m_closeWindowButton.Init();
      m_treeViewPanel.Init();
    }

    void SetToolboxPanel(ToolboxPanel<EC>* toolboxPanel)
    {
      m_toolboxPanel = toolboxPanel;
    }

    virtual void PaintUpdateVisibility(Drawing& d)
    {
      bool visible =
        m_toolboxPanel != NULL &&
        (Panel::IsVisible() ||
        (m_toolboxPanel->IsVisible() &&
         m_toolboxPanel->GetSelectedTool() == TOOL_ATOM_SELECTOR));
      this->SetVisible(visible);
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

    virtual void PaintComponent(Drawing& d)
    {
      this->Super::PaintComponent(d);

      if(!m_atom || !m_grid)
      {
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        const char* message = "No atom selected.";
        d.BlitText(message, UPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
        return;
      }

      OString256 buff;
      AtomSerializer<AC> serializer(*m_atom);

      const u32 ATOM_X_START = 20;
      const u32 ATOM_Y_START = 10;
      const u32 etype = m_atom->GetType();
      const Element<EC>* element = m_grid->LookupElement(etype);

      if (!element)
      {
        buff.Printf("Unknown type '0x%04x' in: %@", etype, &serializer);
        const char * str = buff.GetZString();
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(str, UPoint(32, 32), MakeUnsigned(d.GetTextSize(str)));
        return;
      }

      d.SetForeground(Panel::GetForeground());
      d.FillCircle(ATOM_X_START-1, ATOM_Y_START-1, ATOM_DRAW_SIZE + 2, ATOM_DRAW_SIZE + 2, (ATOM_DRAW_SIZE >> 1) + 1);
      d.SetForeground(element->DefaultPhysicsColor());
      d.FillCircle(ATOM_X_START, ATOM_Y_START, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE >> 1);
      d.SetFont(FONT_ASSET_ELEMENT);
      d.SetForeground(Drawing::WHITE);
      d.SetBackground(Drawing::BLACK);

      /* Center on the draw_size x draw_size box */
      d.BlitBackedTextCentered(element->GetAtomicSymbol(),
                               UPoint(ATOM_X_START, ATOM_Y_START), UPoint(ATOM_DRAW_SIZE, ATOM_DRAW_SIZE));

      UPoint nameSize = MakeUnsigned(d.GetTextSize(element->GetName()));
      d.BlitBackedText(element->GetName(),
                       UPoint(ATOM_X_START + 3*ATOM_DRAW_SIZE/2,
                              ATOM_Y_START + ATOM_DRAW_SIZE - nameSize.GetY()), // bottom align text
                       nameSize);

      buff.Reset();
      element->AppendDescription(m_atom, buff);
      const char* zstr = buff.GetZString();

      d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
      const u32 LINE_X_START = 4 + ATOM_DRAW_SIZE;
      const u32 LINE_Y_START = 28;
      const u32 LINE_HEIGHT = TTF_FontLineSkip(AssetManager::GetReal(d.GetFont()));
      const u32 INDENT_AMOUNT = LINE_HEIGHT;

      d.BlitBackedText(zstr, UPoint(LINE_X_START, LINE_Y_START + 0 * LINE_HEIGHT),
                       MakeUnsigned(d.GetTextSize(zstr)));

      buff.Reset();
      buff.Printf("%@", &serializer);
      zstr = buff.GetZString();

      d.BlitBackedText(zstr, UPoint(LINE_X_START, LINE_Y_START + 1 * LINE_HEIGHT),
                       MakeUnsigned(d.GetTextSize(zstr)));

      const UlamElement<EC> * uelt = element->AsUlamElement();
      if (!uelt)
      {
        PaintDisplayAtomicControllers(d, *m_atom, element);
        return;
      }

      const UlamClassRegistry & ucr = m_grid->GetUlamClassRegistry();

      const u32 printFlags =
        UlamClass::PRINT_MEMBER_NAMES |
        UlamClass::PRINT_MEMBER_VALUES |
        UlamClass::PRINT_RECURSE_QUARKS;

      uelt->Print(ucr, buff, *m_atom, printFlags);
      zstr = buff.GetZString();
      u32 indent = 0;
      u32 lineNum = 1;
      OString64 lineBuff;

      for (u8 ch = *zstr; ch; ch = *++zstr)
      {
        s32 oldIndent = indent;
        s32 oldLineNum = lineNum;
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
            d.BlitBackedText(line, UPoint(LINE_X_START + oldIndent * INDENT_AMOUNT, LINE_Y_START + oldLineNum * LINE_HEIGHT),
                             MakeUnsigned(d.GetTextSize(line)));
            lineBuff.Reset();
          }
        } else
        {
          lineBuff.Printf("%c", ch);
        }
      }
      UPoint ds = Panel::GetDesiredSize();
      u32 dy = MAX(200u,LINE_Y_START + (lineNum + 1) * LINE_HEIGHT);
      if (dy != ds.GetY())
      {
        ds.SetY(dy);
        Panel::SetDesiredSize(ds.GetX(), ds.GetY());
        Panel * parent = this->GetParent();
        if (parent)
          HandleResize(parent->GetDimensions());
      }
    }

    void SetAtom(T* atom)
    {
      if(atom && atom->IsSane())
      {
        m_atom = atom;
      }
      else
      {
        m_atom = NULL;
      }
    }

    void SetGrid(Grid<GC>* grid)
    {
      m_grid = grid;
    }
  };
}

#endif /* ATOMVIEWPANEL_H */
