#include "mouse.h"

void Mouse::Press(Uint8 button)
{
  m_current.insert(button);
}

void Mouse::Release(Uint8 button)
{
  m_current.erase(button);
}

bool Mouse::IsDown(Uint8 button)
{
  return m_current.count(button) > 0;
}

bool Mouse::IsUp(Uint8 button)
{
  return m_current.count(button) == 0;
}

bool Mouse::SemiAuto(Uint8 button)
{
  return m_current.count(button) > 0 &&
    m_prev.count(button) == 0;
}

void Mouse::Flip()
{
  m_prev = m_current;
}
