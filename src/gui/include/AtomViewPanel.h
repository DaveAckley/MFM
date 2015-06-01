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
      // Mon Jun  1 03:33:42 2015 NOT READY FOR PRIME TIME
      // this->Panel::Insert(&m_treeViewPanel, NULL);
    }

    void Init()
    {
      Panel::SetDesiredSize(500, 300);
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
      this->SetVisibility(visible);
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
        d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));
        const char* message = "No atom selected.";
        d.BlitText(message, UPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
      }
      else
      {
        const Element<EC>* element = m_grid->LookupElement(m_atom->GetType());
        d.SetForeground(element->DefaultPhysicsColor());
        d.FillCircle(2, 2, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE >> 1);
        d.SetFont(FONT_ASSET_ELEMENT);
        d.SetForeground(Drawing::WHITE);
        d.SetBackground(Drawing::BLACK);

        /* As long as the font is monospaced, we can get the text size
           of any 2-character string for this centering. */
        const UPoint elementTextSize = MakeUnsigned(d.GetTextSize("12"));
        d.BlitBackedTextCentered(element->GetAtomicSymbol(), UPoint(8, 8), elementTextSize);

        d.BlitBackedText(element->GetName(), UPoint(4 + ATOM_DRAW_SIZE, 2),
                         MakeUnsigned(d.GetTextSize(element->GetName())));

        OString64 desc;
        element->AppendDescription(m_atom, desc);
        const char* zstr = desc.GetZString();

        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        const u32 LINE_X_START = 4 + ATOM_DRAW_SIZE;
        const u32 LINE_Y_START = 28;
        const u32 LINE_HEIGHT = 14;

        d.BlitBackedText(zstr, UPoint(LINE_X_START, LINE_Y_START + 0 * LINE_HEIGHT),
                         MakeUnsigned(d.GetTextSize(zstr)));

        OString64 atomBody;
        AtomSerializer<AC> serializer(*m_atom);
        atomBody.Printf("%@", &serializer);
        zstr = atomBody.GetZString();

        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(zstr, UPoint(LINE_X_START, LINE_Y_START + 1 * LINE_HEIGHT),
                         MakeUnsigned(d.GetTextSize(zstr)));

        const UlamElement<EC> * uelt = element->AsUlamElement();
        if (!uelt)
          PaintDisplayAtomicControllers(d, *m_atom, element);
        else
        {
          const UlamClassRegistry & ucr = m_grid->GetUlamClassRegistry();
          OString128 buff;
          const u32 printFlags =
            UlamClass::PRINT_MEMBER_NAMES |
            UlamClass::PRINT_MEMBER_VALUES |
            UlamClass::PRINT_RECURSE_QUARKS;
          uelt->Print(ucr, buff, *m_atom, printFlags);
          zstr = buff.GetZString();

          d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
          d.BlitBackedText(zstr, UPoint(LINE_X_START, LINE_Y_START + 2 * LINE_HEIGHT),
                           MakeUnsigned(d.GetTextSize(zstr)));

        }
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
