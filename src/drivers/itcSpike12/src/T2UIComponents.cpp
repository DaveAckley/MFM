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

  void T2TileListenCheckbox::OnCheck(bool value) {
    this->SetChecked(value);
  }

  bool T2TileListenCheckbox::IsChecked() const {
    return T2Tile::get().isListening();
  }

  void T2TileListenCheckbox::SetChecked(bool checked) {
    T2Tile::get().setListening(checked);
  }

  void T2SeedPhysicsButton::onClick() {
    T2Tile::get().seedPhysics();
  }

  void T2ClearTileButton::onClick() {
    T2Tile::get().clearPrivateSites();
  }

}
