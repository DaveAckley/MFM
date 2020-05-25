/* -*- C++ -*- */
#ifndef T2UICOMPONENTS_H
#define T2UICOMPONENTS_H

#include "AbstractCheckbox.h"

namespace MFM {

  struct T2TileLiveCheckbox : public AbstractCheckbox {
    virtual void OnCheck(bool value) ;
    virtual bool IsChecked() const ;
    virtual void SetChecked(bool checked) ;
  };

  struct T2TileListenCheckbox : public AbstractCheckbox {
    virtual void OnCheck(bool value) ;
    virtual bool IsChecked() const ;
    virtual void SetChecked(bool checked) ;
  };

  struct T2UIButton : public AbstractButton {
    // AbstractButton methods
    virtual void OnClick(u8 button) { onClick(); }
    virtual s32 GetSection() { return HELP_SECTION_WINDOWS; }
    virtual const char * GetDoc() { return Panel::GetDoc(); }
    virtual bool GetKey(u32& keysym, u32& mods) {
      return this->GetKeyboardAccelerator(keysym, mods);
    }
    virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mod) {
      return false;
    }
    virtual bool ExecuteFunction(u32 keysym, u32 mods) {
      OnClick(SDL_BUTTON_LEFT);
      return true;
    }

    // T2UIButton methods
    virtual void onClick() = 0;
  };
  
  struct T2SeedPhysicsButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2ClearTileButton : public T2UIButton {
    virtual void onClick() ;
  };

}


#endif /* T2UICOMPONENTS_H */
