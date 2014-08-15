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

#include "Panel.h"
#include "Grid.h"

namespace MFM
{
  template <class GC>
  class AtomViewPanel : public Panel
  {
   private:
    typedef typename GC::CORE_CONFIG::ATOM_TYPE T;
    typedef typename GC::CORE_CONFIG CC;

    const T* m_atom;

    Grid<GC>* m_grid;

    static const u32 ATOM_DRAW_SIZE = 16;

   public:
    AtomViewPanel() :
      Panel(300, 100),
      m_atom(NULL),
      m_grid(NULL)
    {
    }

    virtual void PaintComponent(Drawing& d)
    {
      d.SetBackground(Panel::GetBackground());
      d.SetForeground(Panel::GetForeground());
      d.FillRect(Rect(SPoint(0, 0), Panel::GetDimensions()));

      if(!m_atom || !m_grid)
      {
        d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));
        d.SetForeground(Drawing::BLACK);
        const char* message = "No atom selected.";
        d.BlitText(message, UPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
      }
      else
      {
        const Element<CC>* element = m_grid->LookupElement(m_atom->GetType());
        d.SetForeground(element->DefaultPhysicsColor());
        d.FillCircle(2, 2, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE >> 1);
        d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));
      }
    }

    void SetAtom(const T* atom)
    {
      m_atom = atom;
    }

    void SetGrid(Grid<GC>* grid)
    {
      m_grid = grid;
    }
  };
}

#endif /* ATOMVIEWPANEL_H */
