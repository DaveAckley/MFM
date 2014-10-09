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

    virtual void PaintComponent(Drawing& d)
    {
      SDL_Surface* icon = GetIcon();
      d.BlitAsset(GetAsset(), UPoint(0, 0), UPoint(icon->w, icon->h));

      d.SetForeground(Panel::GetForeground());

      d.BlitText(AbstractButton::GetText(), UPoint(32, 0),
                 AbstractButton::GetDimensions());
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
     * Gets the Asset which should be drawn depending on IsChecked()
     *
     * @returns The Asset which should be drawn
     */
    inline Asset GetAsset()
    {
      return IsChecked() ?
        ASSET_CHECKBOX_ICON_ON :
        ASSET_CHECKBOX_ICON_OFF ;
    }

    /**
     * Gets the SDL_Surface pointer which should be drawn depending on
     * the state held by the boolean value pointed to by \c m_externalValue .
     *
     * @returns The SDL_Surface pointer which should be drawn
     *          depending on the state held by the boolean value
     *          pointed to by \c m_externalValue .
     */
    inline SDL_Surface* GetIcon()
    {
      return AssetManager::Get(GetAsset());
    }

    /**
     * To be performed only once during initialization. This is most
     * likely going to be used by constructors to set the default
     * values of some properties of this AbstractCheckbox .
     */
    void OnceOnly()
    {
      Panel::SetForeground(Drawing::WHITE);
    }
  };

  /**
   * A subclass of an AbstractCheckbox in which an extern bool * is
   * used to implement the boolean value.
   */
  class AbstractCheckboxExternal : public AbstractCheckbox
  {
    bool * m_externalPointer;

  public:
    AbstractCheckboxExternal(const char * title) :
      AbstractCheckbox(title),
      m_externalPointer(0)
    {
    }

    /**
     * Sets the pointer to the boolean value which is backing this
     * AbstractCheckbox . It is important to set this before using
     * this AbstractCheckbox .
     *
     * @param externalValue The pointer to the boolean value wished to
     *                      back this AbstractCheckbox . Any
     *                      modifications to the state of this
     *                      AbstractCheckbox will also modify the
     *                      contents of this pointer .
     */
    void SetExternalValue(bool * ptr)
    {
      m_externalPointer = ptr;
    }

    /**
     * Checks to see if this AbstractCheckbox is in its on state. If
     * it is in its on state, its backing boolean value (if it exists)
     * is guaranteed to be true. If it is not, or does not have a
     * backing boolean value, this will be \c false .
     *
     * @returns \c true if the boolean value backed by this
     *          AbstractCheckbox is set and is equal to \true , else
     *          \c false.
     */
    virtual bool IsChecked() const
    {
      if (!m_externalPointer)
      {
        return false;
      }
      return *m_externalPointer;
    }

    /**
     * Sets the value of the boolean backing this AbstractCheckbox
     * . If there is no boolean bacing this AbstractCheckbox , this
     * FAILs with NULL_POINTER .
     *
     * @param checked The new value to set the boolean value backing
     *                this AbstractCheckbox to.
     */
    virtual void SetChecked(bool checked)
    {
      MFM_API_ASSERT_NONNULL(m_externalPointer);
      *m_externalPointer = checked;
    }
  };
}

#endif /* ABSTRACTCHECKBOX_H */
