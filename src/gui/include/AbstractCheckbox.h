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

  /**
   * A subclass of an AbstractButton, used to toggle a boolean value.
   */
  class AbstractCheckbox : public AbstractButton
  {
   public:
    typedef AbstractButton Super;

    /**
     * Constructs a new AbstractCheckbox which is not linked to a
     * boolean value and is therefore not ready to be used.
     */
    AbstractCheckbox() :
      AbstractButton()
    {
      AbstractCheckbox::OnceOnly();
    }

    /**
     * Constructs a new AbstractCheckbox which displays a given string
     * as its text. The resulting AbstractCheckbox still needs to be
     * given an external backing value in order to be used.
     */
    AbstractCheckbox(const char* text) :
      AbstractButton(text)
    {
      AbstractCheckbox::OnceOnly();
    }

    virtual void PaintBorder(Drawing& d)
    { /* Don't want to paint a border*/ }

    virtual void UpdateEnabling() 
    {
      this->SetColorsFromEnabling();
      this->SetIconSlot(this->GetIconSlot());
    }

    /*

    virtual void PaintComponent(Drawing& d)
    {

      d.SetForeground(Panel::GetForeground());
      d.SetBackground(Panel::GetBackground());
      d.Clear();

      ImageAsset icon = GetImageAsset();
      UPoint iconSize = AssetManager::GetSize(icon);
      u32 sy = 0;
      if (Panel::GetHeight() > iconSize.GetY())
        sy = (Panel::GetHeight() - iconSize.GetY()) / 2;

      d.BlitImageAsset(icon, SPoint(0, sy));

      const char * text = AbstractButton::GetText();
      if(strlen(text))
      {
        SPoint dims = MakeSigned(Panel::GetDimensions());
        SPoint norg = SPoint((s32) iconSize.GetX(), 0);
        dims -= norg;
        SPoint textSize = Panel::GetTextSize(d.GetFont(), text);
        SPoint renderAt = max((dims - textSize)/2, SPoint(0,0)) + norg;

        d.BlitText(text, renderAt, GetDimensions());
      }
    }
    */

    virtual s32 GetSection() { return HELP_SECTION_MISC; } // If not overridden
    virtual const char * GetDoc() { return this->Panel::GetDoc(); } // If not overridden
    virtual bool GetKey(u32& keysym, u32& mods) { return false; } // If not overridden
    virtual bool ExecuteFunction(u32 keysym, u32 mods) {
      OnClick(SDL_BUTTON_LEFT);
      return true;
    }

    virtual void OnClick(u8 button)
    {
      SetChecked(!IsChecked());

      OnCheck(IsChecked());
    }

    /**
     * Called when this AbstractCheckbox is clicked. A value is given
     * describing whether or not this AbstractCheckbox is checked.
     *
     * @param value \true if this AbstractCheckbox is in its on state,
     *              else \c false .
     */
    virtual void OnCheck(bool value) = 0;

    /**
     * Checks to see if this AbstractCheckbox is in its on state.
     */
    virtual bool IsChecked() const = 0;

    /**
     * Set this checkbox value to checked.
     */
    virtual void SetChecked(bool checked) = 0;

  private:

    /**
     * Gets the icon slot which should be drawn depending on IsChecked()
     *
     * @returns The MasterIconZSheetSlot of the icon to draw
     */
    inline MasterIconZSheetSlot GetIconSlot()
    {
      return IsChecked() ?
        ZSLOT_ICON_CHECKBOX_ON :
        ZSLOT_ICON_CHECKBOX_OFF ;
    }

    /**
     * To be performed only once during initialization. This is most
     * likely going to be used by constructors to set the default
     * values of some properties of this AbstractCheckbox .
     */
    void OnceOnly()
    {
      SetForeground(Drawing::WHITE);
      SetBackground(Drawing::GREY10);
    }
  };
}

#endif /* ABSTRACTCHECKBOX_H */
