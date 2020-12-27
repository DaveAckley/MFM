/* -*- C++ -*- */
#ifndef T2UICOMPONENTS_H
#define T2UICOMPONENTS_H

#include "AbstractCheckbox.h"
#include "AbstractRadioButton.h"
#include "T2FlashTrafficManager.h"
#include "T2TracePanel.h"

namespace MFM {
  struct MFMRunRadioGroup : public AbstractRadioGroup {
    typedef AbstractRadioGroup Super;
    MFMRunRadioGroup()
      : Super("RP")
    {
      AbstractRadioGroup::Register(*this);
    }

    //@Override
    void OnCheck(AbstractRadioButton & arb, bool value) ;
  };

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
    virtual bool ExecuteFunction(u32 keysym, u32 mods) ;

    // T2UIButton methods
    virtual void onClick() = 0;
  };

  struct T2FlashCommandLabel : T2UIButton {
    // T2UIButton methods
    virtual void onClick() ;
  };

  struct T2HardButton : public T2UIButton {
    T2HardButton() : mDownTime(0) { }

    /* Show eject timer */
    virtual void PaintComponent(Drawing & config) ;

    /** Hard button press / release comes in as keyboard SDLK_MENU events */
    virtual bool Handle(KeyboardEvent & event) ;

    virtual void onClick() ;

    u32 mDownTime;
  };

  struct T2SeedPhysicsButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2TraceCtlButton : public T2UIButton {
    T2TracePanel * mTracePanel;
    T2TraceCtlButton()
      : mTracePanel(0)
    { }
    T2TracePanel & getTracePanel() ;
    virtual void onClick() ;
  };

  struct T2DebugSetupButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2ClearTileButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2QuitButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2CrashButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2DumpButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2OffButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2BootButton : public T2UIButton {
    virtual void onClick() ;
  };

  struct T2KillCDMButton : public T2UIButton {
    virtual void onClick() ;
  };

}


#endif /* T2UICOMPONENTS_H */
