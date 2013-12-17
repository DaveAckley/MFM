#ifndef MOUSE_H
#define MOUSE_H

#include <set>
#include "SDL/SDL.h"

class Mouse
{
private:
  std::set<Uint8> m_current;
  std::set<Uint8> m_prev;

public:

  Mouse() { }

  ~Mouse() { }
  
  void Press(Uint8 button);

  void Release(Uint8 button);

  bool IsDown(Uint8 button);
  
  bool IsUp(Uint8 button);

  bool SemiAuto(Uint8 button);

  /* 
   * This should be called once a
   * frame to keep the SemiAuto
   * function working.
   */
  void Flip();
};

#endif /*MOUSE_H*/
