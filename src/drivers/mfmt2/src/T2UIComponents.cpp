#include "T2UIComponents.h"
#include "T2FlashTrafficManager.h"

#include "T2Tile.h"
#include "TraceTypes.h"

namespace MFM {

  void MFMRunRadioGroup::OnCheck(AbstractRadioButton & arb, bool value) {
    if (!value) return;
    const char * bn = arb.GetName();
    const char * gn = this->GetGroupName().GetZString();
    if (0) { }
    else if (endsWith(bn,"_MFMRun"))   T2Tile::get().setLiving(true);
    else if (endsWith(bn,"_MFMPause")) T2Tile::get().setLiving(false);
    else {
      LOG.Warning("WTC? %s in group %s?", bn, gn);
    }
  }

  void T2FlashCommandLabel::onClick() {
    T2Tile & tile = T2Tile::get();
    tile.getFlashTrafficManager().onClick(this);
  }

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

  void T2HardButton::onClick() {
    const char * name = this->GetName();
    LOG.Error("DON'T CLICK THE HARD BUTTON '%s'",name);
    return;
  }

  void T2HardButton::PaintComponent(Drawing & config) {
    if (mDownTime) {
      u32 nownow = T2Tile::get().now();
      u32 delta = nownow - mDownTime;
      const u32 MAX = 2000;
      if (delta > MAX) {
        mDownTime = 0;
        this->SetText("EJECT");
      } else {
        OString32 buf;
        buf.Printf("%d",MAX-delta);
        this->SetText(buf.GetZString());
      }
      this->AbstractButton::PaintComponent(config);
    }
  }

  bool T2HardButton::Handle(KeyboardEvent & event) {
    if (event.m_event.keysym.sym == SDLK_MENU) {
      Panel * parent = this->GetParent();
      if (event.m_event.type == SDL_KEYDOWN) {
        mDownTime = T2Tile::get().now();
        this->SetVisible(1);
        parent->RaiseToTop(this);
      } else {
        this->SetVisible(0);
        parent->LowerToBottom(this);
        mDownTime = 0;
      }
      return true;
    }
    return false;
  }


  void T2SeedPhysicsButton::onClick() {
    const char * name = this->GetName();
    u32 type = 0;
    if (sscanf(name,"%*[A-Za-z_]%d",&type) != 1) {
      LOG.Error("BADSEED BUTTONNAME '%s'",name);
      return;
    }
    T2FlashCmd cmd;
    switch (type) {
    case 1:  cmd = T2FLASH_CMD(phy,seed1); break;
    case 2:  cmd = T2FLASH_CMD(phy,seed2); break;
    default:
      LOG.Error("BADSEED TYPE '%d'",type);
      return;
    }
    FlashTraffic::execute(FlashTraffic::make(cmd));
  }

  void T2DebugSetupButton::onClick() {
    TLOG(MSG,"DebugtSetup button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(phy,debugsetup)));
  }

  void T2ClearTileButton::onClick() {
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(phy,clear)));
  }

  void T2QuitButton::onClick() {
    TLOG(MSG,"Quit button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(mfm,quit)));
  }

  void T2CrashButton::onClick() {
    TLOG(MSG,"Crash button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(mfm,crash)));
  }

  void T2DumpButton::onClick() {
    TLOG(MSG,"Dump button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(mfm,dump)));
  }

  void T2OffButton::onClick() {
    TLOG(MSG,"Off button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(t2t,off)));
  }

  void T2BootButton::onClick() {
    TLOG(MSG,"Boot button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(t2t,boot)));
  }

  void T2KillCDMButton::onClick() {
    TLOG(MSG,"xCDM button clicked");
    FlashTraffic::execute(FlashTraffic::make(T2FLASH_CMD(t2t,xcdm)));
  }

}
