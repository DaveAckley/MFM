/* -*- C++ -*- */
#include "GridPanel.h"

namespace MFM {

  template <class GC>
  bool AtomViewPanel<GC>::Handle(MouseButtonEvent& mbe) 
  {
    SDL_MouseButtonEvent & event = mbe.m_event.button;
    bool isLeft = event.button == SDL_BUTTON_LEFT;
    bool isRight = event.button == SDL_BUTTON_RIGHT;
    if ((mbe.m_keyboardModifiers == 0) && (isLeft || isRight))
    {
      if(event.type == SDL_MOUSEBUTTONDOWN)
        GetGridPanel().SelectAtomViewPanel(*this); 
      return true;  // Eat all no-mod mouse L and R actions inside us
    }

    return Super::Handle(mbe);
  }

  template <class GC>
  bool AtomViewPanel<GC>::Handle(MouseMotionEvent& mme) 
  {
    if ((mme.m_keyboardModifiers == 0) && GridPanel<GC>::IsLeftOrRightSetInButtonMask(mme.m_buttonMask))
    {
      return true;  // Eat all no-mod motion L and R actions inside us
    }

    return Super::Handle(mme);
  }

  template <class GC>
  bool AtomViewPanel<GC>::Handle(KeyboardEvent& kbe) 
  {
    SDL_KeyboardEvent & key = kbe.m_event;
    if(key.type == SDL_KEYUP)
    {
      if (key.keysym.sym == SDLK_ESCAPE)
      {
        GetGridPanel().UnselectAtomViewPanel(*this); 
        this->ClearAtomCoord();
        this->SetVisible(false);
        return true;
      }
    }
    return Super::Handle(kbe);
  }

}
