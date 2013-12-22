#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <set>
#include "itype.h"
#include "SDL/SDL.h"

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

  bool IsDown(u32 key);

  bool IsUp(u32 key);

  bool SemiAuto(u32 key);

  void Flip();
};

#endif /*KEYBOARD_HPP*/
