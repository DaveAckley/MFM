#include "drawing.h" /* -*- C++ -*- */
#include <stdlib.h>
#include <string.h>

namespace MFM {

  template <class T, u32 R, u32 W, u32 H>
  void StatsRenderer::RenderGridStatistics(Grid<T,R,W,H>& grid, double aeps, double aer)
  { 
    Drawing::FillRect(m_dest, m_drawPoint.GetX(), m_drawPoint.GetY(),
                      m_dimensions.GetX(), m_dimensions.GetY(),
                      0xff400040);

    const u32 STR_BUFFER_SIZE = 128;
    char strBuffer[STR_BUFFER_SIZE];

    sprintf(strBuffer, "%8.3f kAEPS", aeps/1000.0);

    Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 0),
                      Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

    sprintf(strBuffer, "%8.3f AER", aer);

    Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 20),
                      Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

    for (u32 i = 0; i < m_displayTypesInUse; ++i) {
      u32 type = m_displayTypes[i];
      const Element<T,R> * elt = grid.GetTile(SPoint(0,0)).GetElementTable().Lookup(type);
      if (elt == 0) continue;
      u32 typeCount = grid.GetAtomCount(type);

      snprintf(strBuffer, STR_BUFFER_SIZE, "%8d %s", typeCount, elt->GetName());

      Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 20*i+50),
                        Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);
    }
  }
} /* namespace MFM */

