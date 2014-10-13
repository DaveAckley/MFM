#include "StdPanel.h"

namespace MFM
{
  bool StdPanel::Handle(SDL_MouseButtonEvent & event)
  {
    const SPoint at(event.x, event.y);
    switch (event.type) {
    case SDL_MOUSEBUTTONUP: {
      switch (event.button) {
      case SDL_BUTTON_WHEELUP:
        return ScrollUp();
      case SDL_BUTTON_WHEELDOWN:
        return ScrollDown();
      default:
        return Click(at, event.button, 0);  // XXX Get modifiers
      }
      break;
    }
    case SDL_MOUSEBUTTONDOWN:
      FAIL(INCOMPLETE_CODE);
    default:
      FAIL(INCOMPLETE_CODE);
    }
    return false;
  }

}
