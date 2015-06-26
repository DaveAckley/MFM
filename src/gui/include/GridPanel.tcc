/* -*- C++ -*- */

#include "GridTool.h"

namespace MFM {
  template <class GC>
  void GridPanel<GC>::PaintGridOverlays(Drawing & drawing)
  {
    if (m_currentGridTool)
      m_currentGridTool->PaintOverlay(drawing);
  }

}
