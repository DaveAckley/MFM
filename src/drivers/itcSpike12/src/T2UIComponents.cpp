#include "T2UIComponents.h"

#include "T2Tile.h"
#include "TraceTypes.h"

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
    T2Tile & tile = T2Tile::get();
    tile.seedPhysics();
    SDLI & sdli = tile.getSDLI();
    const char * panelName = "GlobalMenu_Button_Sites";
    AbstractButton * sites = dynamic_cast<AbstractButton*>(sdli.lookForPanel(panelName));
    if (!sites) LOG.Error("Couldn't find '%s'",panelName);
    else sites->OnClick(1);
  }

  void T2ClearTileButton::onClick() {
    T2Tile::get().clearPrivateSites();
  }

  void T2QuitButton::onClick() {
    TLOG(MSG,"Quit button clicked");
    T2Tile::get().getSDLI().stop();
  }

}
