/*                                              -*- mode:C++ -*-
  CloseWindowButton.h Button used for closing its parent Panel.
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
  \file CloseWindowButton.h Button used for closing its parent Panel.
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CLOSEWINDOWBUTTON_H
#define CLOSEWINDOWBUTTON_H

#include "AbstractButton.h"

namespace MFM
{
  /**
   * An implementation of AbstractButton used to toggle visibility of
   * its Parent panel.
   */
  class CloseWindowButton : public AbstractButton
  {
   private:

    /**
     * A pointer to the Panel that this CloseWindowButton resides in
     * and therefore should also hide upon clicking.
     */
    Panel* m_parentPanel;

   public:
    /**
     * Constructs an AbstractButton with default parameters and
     * default coloring scheme.
     */
    CloseWindowButton(Panel* parentPanel) :
      AbstractButton(),
      m_parentPanel(parentPanel)
    { }

    void Init()
    {
      SDL_Surface* icon = AssetManager::Get(ASSET_CLOSE_WINDOW_ICON);
      AbstractButton::SetIcon(icon);
      Panel::SetDimensions(icon->w, icon->h);
      Panel::SetRenderPoint(SPoint(2, 2));
      Panel::m_parent = NULL;
      m_parentPanel->Panel::Insert(this, NULL);
    }

    /**
     * Deconstructs this AbstractButton.
     */
    ~CloseWindowButton()
    { }

    /**
     * Pure abstract behavior method. This is called if this
     * AbstractButton is enabled and has decided that it has been
     * clicked on . Overriding this method allows the subclass of this
     * AbstractButton to behave in any way it pleases upon being clicked.
     *
     * @param button The SDL mouse button that has been pressed to
     *               generate this mouse hit, i.e. SDL_BUTTON_LEFT ,
     *               SDL_BUTTON_MIDDLE, or SDL_BUTTON_RIGHT .
     */
    virtual void OnClick(u8 button)
    {
      if(button == SDL_BUTTON_LEFT)
      {
        m_parentPanel->SetVisibility(false);
      }
    }
  };
}

#endif /* CLOSEWINDOWBUTTON_H */
