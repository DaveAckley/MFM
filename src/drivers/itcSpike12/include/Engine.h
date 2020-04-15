/* -*- C++ -*- */
#ifndef ENGINE_H
#define ENGINE_H

#include "SDLI.h"

namespace MFM {
  struct SDLI; // Forward

  struct Event {
    SDL_Event m_sdlEvent;
  };

  struct Engine {
    virtual void input(SDLI&, Event&) = 0;
    virtual void update(SDLI&) = 0;
    virtual void output(SDLI&) = 0;
    virtual ~Engine() { }
  };
}
#endif /* ENGINE_H */
