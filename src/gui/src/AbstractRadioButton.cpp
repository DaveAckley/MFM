#include "AbstractRadioButton.h"

namespace MFM {

  AbstractRadioGroup *(AbstractRadioGroup::m_registeredRadioGroups[MAX_RADIO_GROUPS]);
  void AbstractRadioGroup::Register(AbstractRadioGroup & toBeRegistered)
  {
    for (u32 i = 0; i < MAX_RADIO_GROUPS; ++i)
    {
      if (0 == m_registeredRadioGroups[i])
      {
        m_registeredRadioGroups[i] = &toBeRegistered;
        return;
      }
      AbstractRadioGroup & existing = *m_registeredRadioGroups[i];
      MFM_API_ASSERT_ZERO(existing.GetGroupName().Equals(toBeRegistered.GetGroupName()));
    }
    FAIL(OUT_OF_ROOM);
  }

  AbstractRadioGroup * AbstractRadioGroup::GetAbstractRadioGroupIfAny(const char * named)
  {
    for (u32 i = 0; i < MAX_RADIO_GROUPS; ++i)
    {
      if (0 == m_registeredRadioGroups[i]) break;

      if (m_registeredRadioGroups[i]->GetGroupName().Equals(named))
      {
        return m_registeredRadioGroups[i];
      }
    }
    return 0;
  }

  AbstractRadioButton * AbstractRadioButton::GetPushedIfAny(const char * groupName, Panel * fromPanel)
  {
    MFM_API_ASSERT_NONNULL(fromPanel);
    AbstractRadioButton * arb = dynamic_cast<AbstractRadioButton*>(fromPanel);
    if (arb && arb->m_radioGroup.Equals(groupName) && arb->IsChecked())
    {
      return arb;
    }

    Panel * top = fromPanel->GetTop();
    if (top) {
      Panel * kid = top;
      do {
        kid = kid->GetForward();
        arb = GetPushedIfAny(groupName,kid);
        if (arb) return arb;
      } while (kid != top);
    }

    return 0;
  }

  void AbstractRadioButton::PopRadioGroupFromPanel(const char * zstr, Panel * panel)
  {
    MFM_API_ASSERT_NONNULL(panel);
    AbstractRadioButton * arb = dynamic_cast<AbstractRadioButton*>(panel);
    if (arb && arb->m_radioGroup.Equals(zstr))
    {
      arb->OnCheck(false);  /* Won't reenter PopRadioGroup since value==false*/
    }

    Panel * top = panel->GetTop();
    if (top) {
      Panel * kid = top;
      do {
        kid = kid->GetForward();
        PopRadioGroupFromPanel(zstr, kid);
      } while (kid != top);
    }
  }

  void AbstractRadioButton::PopRadioGroup()
  {
    Panel * start = this;
    while (start->GetParent() != 0) { /*Find (at least local) root*/
      start = start->GetParent();
    }
    PopRadioGroupFromPanel(m_radioGroup.GetZString(), start);
  }

}
