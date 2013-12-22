#ifndef MOUSE_H
#define MOUSE_H

#include <set>
#include "itype.h"
#include "point.h"
#include "SDL/SDL.h"

class Mouse
{
private:
  std::set<u8> m_current;
  std::set<u8> m_prev;

  u16 m_x, m_y;

public:

  Mouse() { }

  ~Mouse() { }

  void HandleButtonEvent(SDL_MouseButtonEvent* e);

  void HandleMotionEvent(SDL_MouseMotionEvent* e);
  
  void Press(u8 button);

  void Release(u8 button);

  bool IsDown(u8 button);
  
  bool IsUp(u8 button);

  bool SemiAuto(u8 button);

  u16 GetX() { return m_x; }

  u16 GetY() { return m_y; }

  void FillPoint(Point<int>* out);

  /* 
   * This should be called once a
   * frame to keep the SemiAuto
   * function working.
   */
  void Flip();
};

#endif /*MOUSE_H*/
