/*                                              -*- mode:C++ -*-
  AbstractCheckbox.h Boolean button
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
  \file AbstractCheckbox.h Boolean button
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTCHECKBOX_H
#define ABSTRACTCHECKBOX_H

#include "AbstractButton.h"
#include "AssetManager.h"

namespace MFM
{

  class AbstractCheckbox : public AbstractButton
  {
  public:

    AbstractCheckbox() :
      AbstractButton(), m_checked(false)
    {
      OnceOnly();
    }

    AbstractCheckbox(const char* text) :
      AbstractButton(text), m_checked(false)
    {
      OnceOnly();
    }

    virtual void PaintBorder(Drawing& d)
    {}

    virtual void PaintComponent(Drawing& d)
    {
      SDL_Surface* icon = GetIcon();
      d.BlitAsset(GetAsset(), UPoint(0, 0), UPoint(icon->w, icon->h));
      d.BlitText(AbstractButton::GetText(), UPoint(32, 0),
		 AbstractButton::GetDimensions());
    }

    void OnClick(u8 button)
    {
      m_checked = !m_checked;

      OnCheck(m_checked);
    }

    virtual void OnCheck(bool value) = 0;

    bool IsChecked()
    {
      return m_checked;
    }

  private:

    bool m_checked;

    inline Asset GetAsset()
    {
      return m_checked ?
	ASSET_CHECKBOX_ICON_ON :
	ASSET_CHECKBOX_ICON_OFF ;
    }

    inline SDL_Surface* GetIcon()
    {
      return AssetManager::Get(GetAsset());
    }

    void OnceOnly()
    {
      Panel::SetForeground(Drawing::WHITE);
    }
  };
}

#endif /* ABSTRACTCHECKBOX_H */
