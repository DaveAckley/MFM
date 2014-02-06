#include "drawing.h" /* -*- C++ -*- */
#include <stdlib.h>
#include <string.h>

namespace MFM {

template <class T, u32 R>
void StatsRenderer::RenderGridStatistics(Grid<T,R>& grid)
{ 
  Drawing::FillRect(m_dest, m_drawPoint.GetX(), m_drawPoint.GetY(),
		    m_dimensions.GetX(), m_dimensions.GetY(),
		    0xff400040);

  char strBuffer[128];

  memset(strBuffer, 0, 128);

  sprintf(strBuffer, "DReg count: %d", grid.GetAtomCount(ELEMENT_DREG));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 0),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Res count: %d", grid.GetAtomCount(ELEMENT_RES));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 20),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Sorter count: %d", grid.GetAtomCount(ELEMENT_SORTER));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 40),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Emitter count: %d", grid.GetAtomCount(ELEMENT_EMITTER));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 60),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Consumer count: %d", grid.GetAtomCount(ELEMENT_CONSUMER));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 80),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Data count: %d", grid.GetAtomCount(ELEMENT_DATA));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 100),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);
  
}
} /* namespace MFM */

