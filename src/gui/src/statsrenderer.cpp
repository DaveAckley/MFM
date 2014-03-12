#include "statsrenderer.h"

namespace MFM {
  bool StatsRenderer::DisplayStatsForType(u32 type) {
    if (m_displayTypesInUse >= MAX_TYPES) return false;
    m_displayTypes[m_displayTypesInUse++] = type;
    return true;
  }

#if 0
  void StatsRenderer::DisplayTypeAt(u32 lineNum, u32 type) {
    char strBuffer[128];
    

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
#endif

}
