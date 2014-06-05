#include "Keyboard.h"

namespace MFM {

  void Keyboard::HandleEvent(SDL_KeyboardEvent* e)
  {
    if(e->type == SDL_KEYUP)
      {
        Release(e->keysym.sym);
      }
    else if(e->type == SDL_KEYDOWN)
      {
        Press(e->keysym.sym);
      }
  }

  void Keyboard::Press(u32 key)
  {
    m_current.insert(key);
  }

  void Keyboard::Release(u32 key)
  {
    m_current.erase(key);
  }

  bool Keyboard::AltHeld() const
  {
    return m_current.count(SDLK_RALT) > 0 ||
      m_current.count(SDLK_LALT) > 0;
  }

  bool Keyboard::ShiftHeld() const
  {
    return m_current.count(SDLK_RSHIFT) > 0 ||
      m_current.count(SDLK_LSHIFT) > 0;
  }

  bool Keyboard::CtrlHeld() const
  {
    return m_current.count(SDLK_LCTRL) > 0 ||
      m_current.count(SDLK_RCTRL) > 0;
  }

  bool Keyboard::IsDown(u32 key) const
  {
    return m_current.count(key) > 0;
  }

  bool Keyboard::IsUp(u32 key) const
  {
    return m_current.count(key) == 0;
  }

  bool Keyboard::SemiAuto(u32 key)
  {
    return m_current.count(key) > 0 &&
      m_prev.count(key) == 0;
  }

  void Keyboard::Flip()
  {
    m_prev = m_current;
  }
} /* namespace MFM */

