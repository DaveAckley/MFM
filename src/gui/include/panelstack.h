#ifndef PANELSTACK_H    /* -*- C++ -*- */
#define PANELSTACK_H

#include "panel.h"
#include "SDL/SDL.h"

class PanelStack
{
private:

  void* m_renderObjs[0x10];

  Panel* m_panels[0x10];

  u32 m_panelCount;

  SDL_Surface* m_dest;

public:

  PanelStack(SDL_Surface* dest);

  void PushPanel(Panel& panel, void* renderObj);

  Panel* PopPanel();

  void Render();
};

#endif /*PANELSTACK_H*/
