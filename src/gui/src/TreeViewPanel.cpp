#include "TreeViewPanel.h"

namespace MFM {
  void TreeViewPanel::PaintComponent(Drawing& d)
  {
    d.SetBackground(Panel::GetBackground());
    d.SetForeground(Panel::GetForeground());
    d.FillRect(Rect(SPoint(0, 0), Panel::GetDimensions()));

    FAIL(INCOMPLETE_CODE);
  }

}
