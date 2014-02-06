#ifndef GRIDPANEL_H /* -*- C++ -*- */
#define GRIDPANEL_H

#include "itype.h"
#include "gridrenderer.h"

namespace MFM {

class GridPanel : public Panel
{
private:
  
  GridRenderer* m_renderer;

  template <class T, u32 R>
  void RenderHeldGrid(void*& rendObj)
  {
    m_renderer->RenderGrid(*((Grid<T,R>*)rendObj));
  }

public:
  
  GridPanel();

  GridPanel(u32 width, u32 height);

  GridPanel(SDL_Surface* dest);

  GridPanel(SDL_Surface* dest, u32 width, u32 height);

  void SetRenderer(GridRenderer* renderer);

  void Render(void* rendObj)
  {
  }
};
} /* namespace MFM */
#endif /* GRIDPANEL_H */

