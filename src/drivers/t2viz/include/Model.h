/* -*- C++ -*- */
#ifndef MODEL_H
#define MODEL_H

#include "SDLI.h"

namespace MFM {
  struct SDLI; // Forward

  struct Event {
    SDL_Event m_sdlEvent;
  };

  struct Model {
    virtual void input(SDLI&, Event&) = 0;
    virtual void update(SDLI&) = 0;
    virtual void output(SDLI&) = 0;
    virtual ~Model() { }
  };
}
#endif /* MODEL_H */
