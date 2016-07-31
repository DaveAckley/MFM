/* -*- C++ -*- */

#include "GridPanel.h"

namespace MFM {

  template<class GC>
  void ReplayPanel<GC>::TransportButton::UpdateEnabling() 
  {
    this->SetEnabled(m_replayPanel.GetGridPanel().AreAnyTilesSelected());
  }

  template<class GC>
  void ReplayPanel<GC>::TransportButton::OnClick(u8 button)
  {
    OurGridPanel& gridPanel = this->m_replayPanel.GetGridPanel();
    OurGrid& grid = gridPanel.GetGrid();
    for (u32 tx = 0; tx < grid.GetWidth(); ++tx)
    {
      for (u32 ty = 0; ty < grid.GetHeight(); ++ty)
      {
        UPoint utc(tx,ty);
        if (gridPanel.IsTileSelected(utc))
        {
          OurTile & tile = grid.GetTile(MakeSigned(utc));
          OurEventHistoryBuffer & ehb = tile.GetEventHistoryBuffer();
          ehb.MoveCursor(m_delta);
        }
      }
    }
  }

}
