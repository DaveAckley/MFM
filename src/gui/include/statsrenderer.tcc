#include "drawing.h" /* -*- C++ -*- */
#include <stdlib.h>
#include <string.h>

#include "element_dreg.h"
#include "element_res.h"
#include "element_consumer.h"
#include "element_emitter.h"
#include "element_sorter.h"

namespace MFM {

  template <class T, u32 R, u32 W, u32 H>
  void StatsRenderer::RenderGridStatistics(Grid<T,R,W,H>& grid, double aeps, double aer)
{ 
  Drawing::FillRect(m_dest, m_drawPoint.GetX(), m_drawPoint.GetY(),
		    m_dimensions.GetX(), m_dimensions.GetY(),
		    0xff400040);

  char strBuffer[128];

  memset(strBuffer, 0, 128);

  sprintf(strBuffer, "DReg %d", grid.GetAtomCount(Element_Dreg<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 0),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Res %d", grid.GetAtomCount(Element_Res<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 20),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Sorter %d", grid.GetAtomCount(Element_Sorter<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 40),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Emitter %d", grid.GetAtomCount(Element_Emitter<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 60),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Consumer %d", grid.GetAtomCount(Element_Consumer<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 80),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "Data: %d", grid.GetAtomCount(Element_Data<T,R>::TYPE));

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 100),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "AEPS: %g", aeps);

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 140),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);

  sprintf(strBuffer, "AER: %g/s", aer);

  Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), 160),
		    Point<u32>(m_dimensions.GetX(), 20), 0xffffffff);
  
}
} /* namespace MFM */

