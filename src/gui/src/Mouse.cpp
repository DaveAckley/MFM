#include "Mouse.h"

namespace MFM {

  void Mouse::HandleButtonEvent(SDL_MouseButtonEvent* e)
  {
    if(e->state == SDL_PRESSED)
      {
        Press(e->button);
      }
    else if(e->state == SDL_RELEASED)
      {
        Release(e->button);
      }
  }

  void Mouse::HandleMotionEvent(SDL_MouseMotionEvent* e)
  {
    m_x = e->x;
    m_y = e->y;
  }

  void Mouse::Press(u8 button)
  {
    m_current.insert(button);
  }

  void Mouse::Release(u8 button)
  {
    m_current.erase(button);
  }

  bool Mouse::IsDown(u8 button)
  {
    return m_current.count(button) > 0;
  }

  bool Mouse::IsUp(u8 button)
  {
    return m_current.count(button) == 0;
  }

  bool Mouse::SemiAuto(u8 button)
  {
    return m_current.count(button) > 0 &&
      m_prev.count(button) == 0;
  }

  void Mouse::FillPoint(SPoint* out)
  {
    out->Set(m_x, m_y);
  }

  void Mouse::Flip()
  {
    m_prev = m_current;
  }
} /* namespace MFM */

