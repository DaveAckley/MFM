/* -*- C++ -*- */
#ifndef MENU_H
#define MENU_H

#include <map>
#include <string>
#include <vector>

#include "itype.h"
#include "AbstractButton.h"

#include "SDL.h"

#include "T2Constants.h"

namespace MFM {
  struct T2Tile; // FORWARD
  struct MenuMakerPanel; // FORWARD

  struct MenuActionInfo {
    MenuActionInfo(SDLKey key, bool targ)
      : mKey(key)
      , mTargetPanelRequired(targ)
    { }
    SDLKey mKey;
    bool mTargetPanelRequired;
  };
  typedef std::map<std::string,MenuActionInfo> ActionMap;
  typedef std::pair<std::string,MenuActionInfo> ActionMapEntry;
  struct MenuActionMap : public ActionMap {
    MenuActionMap() {
      insert(ActionMapEntry("CALL",   MenuActionInfo(SDLK_RIGHT,true)));
      insert(ActionMapEntry("RETURN", MenuActionInfo(SDLK_LEFT, false)));
      insert(ActionMapEntry("GO",     MenuActionInfo(SDLK_DOWN, true)));
      insert(ActionMapEntry("DISMISS",MenuActionInfo(SDLK_UP,   false)));
    }
  };

  struct MenuAction {
    static const MenuActionMap mActionMap;
    bool parse(ByteSource& bs) ;
    void performAction(u8 button) ;
    SDLKey mKeyToPress;
    OString32 mTargetPanel;
    bool mPanelRequired;
  };

  struct MenuItem : public AbstractButton {
    MenuItem()
      : mAction()
    { }

    virtual s32 GetSection() { return HELP_SECTION_WINDOWS; }
    virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mod) {
      return false;
    }

    virtual const char * GetDoc() { return Panel::GetDoc(); }
    virtual bool GetKey(u32& keysym, u32& mods) {
      return this->GetKeyboardAccelerator(keysym, mods);
    }

    virtual bool ExecuteFunction(u32 keysym, u32 mods) {
      OnClick(SDL_BUTTON_LEFT);
      return true;
    }

    virtual void OnClick(u8 button) {
      performAction(button);
    }

    void performAction(u8 button) {
      mAction.performAction(button);
    }

    void parseAction(const char * action) {
      MFM_API_ASSERT_NONNULL(action);
      CharBufferByteSource cbbs(action,strlen(action));
      if (!mAction.parse(cbbs)) {
        LOG.Error("Can't parse '%s' as menu action",action);
        FAIL(ILLEGAL_ARGUMENT);
      }
    }
  private:
    MenuAction mAction;
  };

  struct Menu : public Panel {
    static const u32 MAX_MENU_ITEM_COUNT = 9;
    MenuItem& getMenuItem(u32 which) ;
    Menu() ;
    void configureMenu(MenuMakerPanel & forOwner) ;
    virtual ~Menu() { }
    void setPixelInset(u32 pix) {
      mInsetPixels = pix;
    }
    void add(MenuItem& toNextAvail) {
      DIE_UNIMPLEMENTED();
    }
  private:
    MenuItem mMenuItems[MAX_MENU_ITEM_COUNT];
    u32 mInsetPixels;
  };

  struct MenuMakerPanel : public Panel {
    /** Add any MenuItems specific to this MenuMakerPanel */
    virtual void addItems(Menu& menu) = 0;

    /** Add this MenuMakerPanel as a submenu item */
    void configureItem(MenuItem& menu) ;

    /** Configure entire menu according to this */
    void configureMenu(Menu& menu) ;

    /** Check for request to pop-up menu */
    virtual bool Handle(KeyboardEvent & event) ;

    MenuMakerPanel()
      : mPosted(false)
    {
    }

    virtual ~MenuMakerPanel() { }

    /** Find the chooser panel and toggle that*/
    bool toggleGlobalPosting() ;

    bool togglePosting() {
      setPosting(!mPosted);
      return mPosted;
    }

    bool setPosting(bool posted) ;

    bool isPosted() const { return mPosted; }

  private:
    bool mPosted;
  };

  struct MenuManager {
    MenuManager(T2Tile& tile)
      : mTile(tile)
    { }
    // Commands understood:
    // CALL PANEL -- SDLK_RIGHT -- Deactivate/push current and raise/activate PANEL
    // RETURN     -- SDLK_LEFT  -- Pop stack and raise/activate previous.
    // GO PANEL   -- SDLK_DOWN  -- Clear stack and raise/activate PANEL
    // DISMISS    -- SDLK_UP    -- Do POP until stack empty
    void execute(const MenuAction& ma, Panel* hitPanel) ;
    Panel* firstVisibleKidOf(Panel * parent) ;
    void makeSingleVisible(Panel * panel) ;
    void flushStack() ;
    void logStack(const char * pfx) ;
    bool toggleGlobalPosting() ;
  private:
    T2Tile & mTile;
    typedef std::vector<Panel*> PanelStack; // NO Panel* OWNERSHIP TAKEN
    PanelStack mPanelStack; 
  };
    
}
#endif /* MENU_H */
