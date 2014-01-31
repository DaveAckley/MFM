#include "drawing.h" /* -*- C++ -*- */
#include <string.h>

template <class T, u32 R>
void StatsRenderer::RenderGridStatistics(Grid<T,R>& grid)
{ 
  Drawing::FillRect(m_dest, m_drawPoint.GetX(), m_drawPoint.GetY(),
		    m_dimensions.GetX(), m_dimensions.GetY(),
		    0xff400040);

  char strBuffer[128];

  memset(strBuffer, 0, 128);

  

  sprintf(strBuffer, "Res count: %d", grid.GetAtomCount(2));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 0),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);
  
  /*
  Drawing::BlitText(m_dest, m_drawFont, "T", Point<u32>(m_drawPoint.GetX(),
							m_drawPoint.GetY() + 32),
		    Point<u32>(m_dimensions.GetX() - 10, 32), 0xffffffff);
  */
  
}
