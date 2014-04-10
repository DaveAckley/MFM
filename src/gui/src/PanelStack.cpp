#include "PanelStack.h"

namespace MFM {

PanelStack::PanelStack(SDL_Surface* dest)
{
  m_panelCount = 0;
  m_dest = dest;
}

void PanelStack::PushPanel(Panel& panel, void* renderObj)
{
  panel.SetDestination(m_dest);
  m_renderObjs[m_panelCount] = renderObj;
  m_panels[m_panelCount++] = &panel;
}

Panel* PanelStack::PopPanel()
{
  return m_panels[--m_panelCount];
}

void PanelStack::Render()
{
  for(u32 i = 0; i < m_panelCount; i++)
  {
    m_panels[i]->Render(m_renderObjs[i]);
  }
}
} /* namespace MFM */

