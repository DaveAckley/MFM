#include "T2UIComponents.h"

#include "T2Tile.h"
#include "TraceTypes.h"

namespace MFM {
  bool T2UIButton::ExecuteFunction(u32 keysym, u32 mods) {
    TLOG(DBG,"%s",__PRETTY_FUNCTION__);
    OnClick(SDL_BUTTON_LEFT);
    return true;
  }

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
    const char * name = this->GetName();
    u32 type = 0;
    if (sscanf(name,"%*[A-Za-z_]%d",&type) != 1) {
      LOG.Error("BADSEED BUTTONNAME '%s'",name);
      return;
    }
    T2Tile & tile = T2Tile::get();
    tile.seedPhysics(type);
    SDLI & sdli = tile.getSDLI();
    const char * panelName = "GlobalMenu_Button_Sites";
    AbstractButton * sites = dynamic_cast<AbstractButton*>(sdli.lookForPanel(panelName));
    if (!sites) LOG.Error("Couldn't find '%s'",panelName);
    else sites->OnClick(1);
  }

  void T2DebugSetupButton::onClick() {
    T2Tile & tile = T2Tile::get();
    tile.debugSetup();
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

  void T2CrashButton::onClick() {
    TLOG(MSG,"Crash button clicked");
    T2Tile::get().stopTracing();
    exit(1); /* 'crash' */
  }

  void T2OffButton::onClick() {
    TLOG(MSG,"Off button clicked");
    T2Tile::get().stopTracing();
    system("poweroff");
  }

  void T2BootButton::onClick() {
    TLOG(MSG,"Boot button clicked");
    T2Tile::get().stopTracing();
    system("reboot");
  }

}
