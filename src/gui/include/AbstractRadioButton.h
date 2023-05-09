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
  class AbstractRadioButton; // FORWARD
  
  /** Collective behavior for an entire radio group.  Subclass this to
      define OnCheck, give it a unique name and call
      AbstractRadioGroup::Register(*this) in its ctor, then your
      OnCheck method will be called whenever any button in the group
      changes state.

      (You may instead/also subclass individual AbstractRadioButtons
      and add behavior by overriding their OnCheck methods.  That will
      defeat this mechanism unless the override method calls
      Super::OnCheck appropriately.)
   */
  class AbstractRadioGroup
  {
  private:
    static const u32 MAX_RADIO_GROUPS = 32;
    static AbstractRadioGroup *(m_registeredRadioGroups[MAX_RADIO_GROUPS]);

    const OString32 m_groupName;

  public:
    static void Register(AbstractRadioGroup & toBeRegistered) ;
    static AbstractRadioGroup * GetAbstractRadioGroupIfAny(const char * named) ;

    static AbstractRadioButton * GetButtonMatchingIfAny(const char * groupname, const char * suffix, Panel * fromPanel) ;

    AbstractRadioGroup(const char * nonNullGroupName)
      : m_groupName(nonNullGroupName)
    { }

    virtual void OnCheck(AbstractRadioButton & arb, bool value) = 0;
    const OString32 & GetGroupName() const { return m_groupName; }
    
  };

  class AbstractRadioButton : public AbstractCheckbox
  {
   private:
    typedef AbstractCheckbox Super;

    OString32 m_radioGroup;
    u32 m_bgPushed;
    u32 m_fgPushed;
    u32 m_bgPopped;
    u32 m_fgPopped;

  public:

    void PopRadioGroup() ;

    static void PopRadioGroupFromPanel(const char * zstr, Panel * fromPanel) ;

    static AbstractRadioButton * GetPushedIfAny(const char * groupName, Panel * fromPanel) ;

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

    const OString32 & GetRadioGroupName() {
      return m_radioGroup;
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
      AbstractRadioGroup * arg =
        AbstractRadioGroup::GetAbstractRadioGroupIfAny(m_radioGroup.GetZString());
      if (arg) arg->OnCheck(*this, value);
    }

    virtual ~AbstractRadioButton() { }
  };
} /* namespace MFM */

#endif /* ABSTRACTRADIOBUTTON_H */
