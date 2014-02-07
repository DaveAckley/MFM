#ifndef KEYBOARD_H      /* -*- C++ -*- */
#define KEYBOARD_H

#include <set>
#include "itype.h"
#include "SDL/SDL.h"

namespace MFM {

class Keyboard
{
private:
  std::set<u32> m_current;
  std::set<u32> m_prev;

public:

  Keyboard() { }

  ~Keyboard() { }

  void HandleEvent(SDL_KeyboardEvent* e);

  void Press(u32 key);

  void Release(u32 key);

  bool ShiftHeld();

  bool CtrlHeld();

  bool IsDown(u32 key);

  bool IsUp(u32 key);

  bool SemiAuto(u32 key);

  void Flip();
};
} /* namespace MFM */
#endif /*KEYBOARD_H*/

