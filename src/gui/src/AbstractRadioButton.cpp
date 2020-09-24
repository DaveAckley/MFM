#include "AbstractRadioButton.h"

namespace MFM {

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
