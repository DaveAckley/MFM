#include "Menu.h"
#include "ChooserPanel.h"
#include "T2Tile.h"

namespace MFM {

  const MenuActionMap MenuAction::mActionMap;

  bool MenuAction::parse(ByteSource & bs) {
    OString32 verb, panel;
    if (bs.Scanf("%w%[^ ]",&verb)!=2) {
      LOG.Error("Missing verb in MenuAction");
      return false;
    }

    bs.Scanf("%w%[^ ]",&panel); // panel is optional

    std::string sverb = verb.GetZString();
    if (mActionMap.find(sverb) == mActionMap.end()) {
      LOG.Error("Unknown verb '%s' in MenuAction", verb.GetZString());
      return false;
    }
    const MenuActionInfo & mai = mActionMap.at(sverb);
    if (mai.mTargetPanelRequired && panel.GetLength()==0) {
      LOG.Error("Panel required for MenuAction verb '%s'", verb.GetZString());
      return false;
    }
    mKeyToPress = mai.mKey;
    mTargetPanel = panel;
    mPanelRequired = mai.mTargetPanelRequired;
    //LOG.Message("MenuAction o%03o '%c' to panel '%s'", mKeyToPress, mKeyToPress, mTargetPanel.GetZString());
    return true;
  }


  void MenuAction::performAction(u8 button) {
    T2Tile & tile = T2Tile::get();
    SDLI& sdli = tile.getSDLI();
    Panel * panel = sdli.lookForPanel(mTargetPanel.GetZString());
    if (mPanelRequired && panel == 0) {
      LOG.Error("Required panel missing, ignoring MenuAction key '%c'", mKeyToPress);
      return;
    }

    MenuManager & mm = sdli.getMenuManager();
    mm.execute(*this, panel);
  }

#if 0
        SDL_KeyboardEvent skey;
        memset(&skey,0,sizeof(skey));
        skey.type = SDL_KEYDOWN;
        skey.state = SDL_PRESSED;
        skey.keysym.sym = mKeyToPress;
        KeyboardEvent key(skey,SPoint(0,0));
        panel->Handle(key);
#endif


  Menu::Menu()
    : mInsetPixels(0)
  {
    // Name and link panels now, configure geometry later
    this->SetName("Global_Popup_Menu");
    MenuItem * prev = 0;
    for (u32 i = 0; i < MAX_MENU_ITEM_COUNT; ++i) {
      OString128 buf;
      MenuItem& mi = getMenuItem(i);
      buf.Reset();
      buf.Printf("Global_Popup_Menu_Item_%D",i);
      mi.SetName(buf.GetZString());
      this->Insert(&mi, prev);
      prev = &mi;
    }
  }

  void Menu::configureMenu(MenuMakerPanel & forOwner) {
    UPoint odim = forOwner.GetDimensions();
    UPoint inset(mInsetPixels,mInsetPixels);
    UPoint udim = odim-inset; // F's up if inset bigger than odim

    this->SetDimensions(udim.GetX(),udim.GetY()); 
    this->SetDesiredSize(udim.GetX(),udim.GetY());
    this->SetRenderPoint(MakeSigned(udim));

    UPoint midim = udim/3;
    for (u32 i = 0; i < MAX_MENU_ITEM_COUNT; ++i) {
      MenuItem & mi = getMenuItem(i);
      // Configure menuitem geometry
      mi.SetDimensions(MakeSigned(midim));
      mi.SetDesiredSize(midim.GetX(),midim.GetY());
      mi.SetRenderPoint(SPoint((i%3)*midim.GetX(),(i/3)*midim.GetY()));

      // Reset to defaults and set disabled
      mi.SetEnabled(false);
      mi.SetText("");
      mi.SetIconSlot(ZSLOT_NONE);
      mi.SetEnabledBg(Drawing::YELLOW);
      mi.SetEnabledFg(Drawing::GREY10);
    }

    // Pop off whereever we were
    Panel * parent = GetParent();
    if (parent != 0) parent->Remove(this);
      
    forOwner.Insert(this,0);
    forOwner.RaiseToTop(this);
  }
      
  MenuItem& Menu::getMenuItem(u32 which) {
    if (which >= MAX_MENU_ITEM_COUNT) FAIL(ILLEGAL_ARGUMENT);
    return mMenuItems[which];
  }

  void MenuMakerPanel::configureMenu(Menu& menu) {

    menu.configureMenu(*this);  // set up menu geometry and stacking for us

    // First, count entries from parents
    u32 entries = 0;
    Panel * p;
    for (p = this; p != 0; p = p->GetParent()) {
      MenuMakerPanel * mm = dynamic_cast<MenuMakerPanel*>(p);
      if (!mm) continue;
      ++entries;
    }
    u32 i = entries;
    // Second, configure parent entries backwards
    for (p = this; p != 0; p = p->GetParent()) {
      MenuMakerPanel * mm = dynamic_cast<MenuMakerPanel*>(p);
      if (!mm) continue;
      mm->configureItem(menu.getMenuItem(--i));
    }

    this->addItems(menu); // Configure any local MenuItem
  }

  void MenuMakerPanel::configureItem(MenuItem& menu) {
    DIE_UNIMPLEMENTED();
  }

#if 0
  bool MenuMakerPanel::toggleGlobalPosting() {
    Panel * top = GetTop();
    if (top != 0) {
      Panel * kid = top;
      do {
        kid = kid->GetForward();
        ChooserPanel * cp = dynamic_cast<ChooserPanel*>(kid);
        if (cp!=0) {
          return cp->togglePosting();
        }
      } while (kid != top);
    }
    FAIL(ILLEGAL_STATE);
  }
#endif
  bool MenuMakerPanel::toggleGlobalPosting() {
    T2Tile & tile = T2Tile::get();
    SDLI& sdli = tile.getSDLI();
    MenuManager & mm = sdli.getMenuManager();
    return mm.toggleGlobalPosting();
  }
    
  bool MenuMakerPanel::setPosting(bool posted) {
    if (posted == mPosted) return false;
    Panel * parent = GetParent();
    MFM_API_ASSERT_NONNULL(parent);
    mPosted = !mPosted;
    if (mPosted) {
      parent->RaiseToTop(this);
      this->SetVisible(true);
    } else {
      parent->LowerToBottom(this);
      this->SetVisible(false);
    }
    LOG.Message("MENUMAKER PANEL %sPOSTED", mPosted ? "" : "UN");
    return mPosted;
  }

  bool MenuMakerPanel::Handle(KeyboardEvent & event) {
    /* Thu Jul 30 04:20:11 2020 Repurposing hard button / SDLK_MENU

    if (event.m_event.type == SDL_KEYDOWN &&
        event.m_event.keysym.sym == SDLK_MENU) {
      toggleGlobalPosting();
      return true;
    }
    */
    if (event.m_event.type == SDL_KEYDOWN &&
        event.m_event.keysym.sym == SDLK_g) {
      setPosting(true);
      return true;
    }
    if (event.m_event.type == SDL_KEYDOWN &&
        event.m_event.keysym.sym == SDLK_ESCAPE) {
      setPosting(false);
      return true;
    }
    return false;
  }

  void MenuManager::makeSingleVisible(Panel * panel) {
    MFM_API_ASSERT_NONNULL(panel);
    Panel * parent = panel->GetParent();
    if (parent == 0) {
      LOG.Error("Attempt to solo unparented panel '%s', ignored",
                panel->GetName());
      return;
    }
    parent->RaiseToTop(panel);
    Panel * top = parent->GetTop();
    if (top != 0) {
      Panel * kid = top;
      do {
        kid = kid->GetForward();
        if (kid==panel) {
          kid->SetVisible(true);
          TimeoutAble * ta = dynamic_cast<TimeoutAble*>(kid);
          if (ta != 0) ta->bump(); // wake dis mofo
        } else
          kid->SetVisible(false);
      } while (kid != top);
    }
  }

  void MenuManager::flushStack() {
    while (mPanelStack.size() > 0) {
      Panel * pop = mPanelStack.back();
      mPanelStack.pop_back();
      makeSingleVisible(pop);
    }
  }

  void MenuManager::logStack(const char * pfx) {
    MFM_API_ASSERT_NONNULL(pfx);
    u32 i = 0;
    LOG.Message("--LOGSTACK: %s--",pfx);
    for (auto itr = mPanelStack.begin(); itr != mPanelStack.end(); ++itr) {
      Panel * p = *itr;
      LOG.Message("%s MM[%d] %s",pfx, i++, p == 0 ? "null" : p->GetName());
    }
  }

  Panel* MenuManager::firstVisibleKidOf(Panel * parent) {
    MFM_API_ASSERT_NONNULL(parent);
    Panel * top = parent->GetTop();
    if (top != 0) {
      Panel * kid = top;
      do {
        if (kid->IsVisible()) {
          return kid;
          break;
        }
        kid = kid->GetForward();
      } while (kid != top);
    }
    return 0;
  }

  bool MenuManager::toggleGlobalPosting() {
    if (mPanelStack.size() > 0) { // Already posted
      flushStack(); // So flush it
      return false;
    } 

    // Find visible kid of root to pop back to later
    T2Tile & tile = T2Tile::get();
    SDLI& sdli = tile.getSDLI();

    Panel * menu = sdli.lookForPanel("GlobalMenu");
    MFM_API_ASSERT_NONNULL(menu);
    
    MenuAction ma;
    ma.mKeyToPress = SDLK_RIGHT; /*CALL*/
    
    this->execute(ma, menu);
    return true;
  }

  void MenuManager::execute(const MenuAction& ma, Panel * hitPanel) {
    switch (ma.mKeyToPress) {
      
    default: {
      LOG.Error("Unimplemented menuaction key %d/'%c' ignored",
                ma.mKeyToPress, ma.mKeyToPress);
      break;
    }

    case SDLK_DOWN: /*GO PANEL*/
      MFM_API_ASSERT_NONNULL(hitPanel);
      flushStack();
      makeSingleVisible(hitPanel);
      break;

    case SDLK_RIGHT: /*CALL PANEL*/
      { 
        MFM_API_ASSERT_NONNULL(hitPanel);
        Panel * parent = hitPanel->GetParent();
        MFM_API_ASSERT_NONNULL(parent);
        Panel * save = firstVisibleKidOf(parent);
        mPanelStack.push_back(save);
        makeSingleVisible(hitPanel);
        break;
      }

    case SDLK_LEFT: /*RETURN*/
      { 
        MFM_API_ASSERT_NULL(hitPanel);
        Panel * restore;
        if (mPanelStack.size() > 0) {
          restore = mPanelStack.back();
          mPanelStack.pop_back();
        } else {
          LOG.Warning("MenuAction RETURN on empty stack");
          T2Tile & tile = T2Tile::get();
          SDLI& sdli = tile.getSDLI();
          restore = sdli.lookForPanel("GlobalMenu");
          MFM_API_ASSERT_NONNULL(restore);
        }
        makeSingleVisible(restore);
        break;
      }

    case SDLK_UP:   /*DISMISS*/
      flushStack();
      break;
    }
  }
}
