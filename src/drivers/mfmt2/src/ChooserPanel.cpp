#include "ChooserPanel.h"

namespace MFM {

  ChooserPanel::ChooserPanel()
  {
  }
  
  void ChooserPanel::addItems(Menu& menu) {
    DIE_UNIMPLEMENTED();
  }

  void ChooserPanel::configureItem(MenuItem& menu) {
    DIE_UNIMPLEMENTED();
  }


#if 0
  void ChooserPanel::configureMenu(Menu& menu) {
    // First, count entries from parents
    u32 entries = 0;
    Panel * p;
    for (p = this; p != 0; p = p->GetParent()) {
      MenuMakerPanel * mm = dynamic_cast<MenuMakerPanel*>(p);
      if (!mm) continue;
      ++entries;
    }
    u32 i = entries;
    // Second, configure parent entries
    for (p = this; p != 0; p = p->GetParent()) {
      MenuMakerPanel * mm = dynamic_cast<MenuMakerPanel*>(p);
      if (!mm) continue;
      mm->configureItem(menu.getMenuItem(--i));
    }

  }
#endif

}
