#include "gridpanel.h"

GridPanel::GridPanel() : Panel()
{

}

GridPanel::GridPanel(u32 width, u32 height) : Panel(width, height)
{
  
}

GridPanel::GridPanel(SDL_Surface* dest) : Panel(dest)
{

}

GridPanel::GridPanel(SDL_Surface* dest, u32 width, u32 height) :
  Panel(dest, width, height)
{

}

void GridPanel::SetRenderer(GridRenderer* renderer)
{
  m_renderer = renderer;
}
