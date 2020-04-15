#include "T2UIComponents.h"

#include "T2Tile.h"

namespace MFM {

  void T2TileLiveCheckbox::OnCheck(bool value) {
    this->SetChecked(value);
  }

  bool T2TileLiveCheckbox::IsChecked() const {
    return T2Tile::get().isLiving();
  }

  void T2TileLiveCheckbox::SetChecked(bool checked) {
    T2Tile::get().setLiving(checked);
  }

}
