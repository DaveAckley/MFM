/*                                              -*- mode:C++ -*-
  AbstractRadioButton.h Base class for groups of checkboxes
  Copyright (C) 2020 Living Computation Foundation.  All rights reserved.

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
  \file AbstractRadioButton.h Base class for groups of checkboxes
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTRADIOBUTTON_H
#define ABSTRACTRADIOBUTTON_H

#include "AbstractCheckbox.h"
#include "Panel.h"

namespace MFM
{
  class AbstractRadioButton : public AbstractCheckbox
  {
   private:
    typedef AbstractCheckbox Super;

    OString32 m_radioGroup;
    u32 m_bgPushed;
    u32 m_fgPushed;
    u32 m_bgPopped;
    u32 m_fgPopped;

    void PopRadioGroup() ;

    static void PopRadioGroupFromPanel(const char * zstr, Panel * fromPanel) ;

  public:

    AbstractRadioButton()
      : Super()
      , m_radioGroup("")
      , m_bgPushed(Drawing::WHITE)
      , m_fgPushed(Drawing::BLACK)
      , m_bgPopped(Drawing::GREY20)
      , m_fgPopped(Drawing::GREY80)
    {
      AbstractRadioButton::OnceOnly();
    }

    void SetBgPushed(u32 color) { m_bgPushed = color; }
    void SetFgPushed(u32 color) { m_fgPushed = color; }
    void SetBgPopped(u32 color) { m_bgPopped = color; }
    void SetFgPopped(u32 color) { m_fgPopped = color; }

    u32 GetBgPushed() const { return m_bgPushed; }
    u32 GetFgPushed() const { return m_fgPushed; }
    u32 GetBgPopped() const { return m_bgPopped; }
    u32 GetFgPopped() const { return m_fgPopped; }

    AbstractRadioButton(const char* text)
      : Super(text)
    {
      AbstractRadioButton::OnceOnly();
    }

    void OnceOnly()
    {
      this->SetIconSlot(ZSLOT_NONE);
    }

    void SetRadioGroup(const char * groupName)
    {
      MFM_API_ASSERT_NONNULL(groupName);
      m_radioGroup.Reset();
      m_radioGroup.Print(groupName);
    }

    /*Override because we don't understand buttons and checkboxes and
      labels oh my grr */
    virtual void PaintComponentNonClick(Drawing& d)
    {
      if (this->IsChecked()) {
        this->SetEnabledFg(m_fgPushed);
        this->SetEnabledBg(m_bgPushed);
      } else {
        this->SetEnabledFg(m_fgPopped);
        this->SetEnabledBg(m_bgPopped);
      }
      Super::PaintComponentNonClick(d);
    }

    /*Override AbstractCheckbox method to avoid setting icons grr*/
    virtual void UpdateEnabling()
    {
      /* don't really want this either? this->SetColorsFromEnabling(); */
    }

    virtual void OnCheck(bool value)
    {
      if (value) PopRadioGroup();
    }

    virtual ~AbstractRadioButton() { }
  };
} /* namespace MFM */

#endif /* ABSTRACTRADIOBUTTON_H */
