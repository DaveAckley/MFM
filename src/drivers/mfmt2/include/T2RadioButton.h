/* -*- C++ -*- */
#ifndef T2RADIOBUTTON_H
#define T2RADIOBUTTON_H

#include "itype.h"
#include "AbstractRadioButton.h"

#include "SDL.h"

#include "T2Constants.h"

namespace MFM {

  struct T2RadioButton : public AbstractRadioButton {
    typedef AbstractRadioButton Super;
    T2RadioButton()
      : Super()
      , m_isChecked(false)
    { }

    virtual bool IsChecked() const
    {
      return m_isChecked;
    }

    virtual void OnCheck(bool checked)
    {
      Super::OnCheck(checked);  /*First maybe pop the group*/
      this->SetChecked(checked);
    }

    virtual void SetChecked(bool checked)
    {
      m_isChecked = checked;
    }

    virtual s32 GetSection() { return HELP_SECTION_WINDOWS; }
    virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mod) {
      return false;
    }

    virtual const char * GetDoc() { return Panel::GetDoc(); }
    /*
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
        LOG.Error("Can't parse '%s' as T2RadioButton action",action);
        FAIL(ILLEGAL_ARGUMENT);
      }
    }
    */
  private:

    /*Our radio buttons are passive (I think) so we'll just manipulate
      a dumb bool and let others look at us */
    bool m_isChecked; 
  };
    
}
#endif /* T2RADIOBUTTON_H */
