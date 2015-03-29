/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"

namespace MFM {

  template <class EC>
  UlamContext<EC>::UlamContext() : m_tile(0) { }

  template <class EC>
  void UlamContext<EC>::SetTile(Tile<EC> & t)
  {
    MFM_API_ASSERT_STATE(!m_tile);
    m_tile = & t;
  }

  template <class EC>
  void UlamContext<EC>::AssertTile()
  {
    MFM_API_ASSERT_STATE(m_tile);
  }

  template <class EC>
  Tile<EC> & UlamContext<EC>::GetTile()
  {
    AssertTile();
    return * m_tile;
  }

  template <class EC>
  Random & UlamContext<EC>::GetRandom()
  {
    AssertTile();
    return GetTile().GetRandom();
  }

  template <class EC>
  EventWindow<EC> & UlamContext<EC>::GetEventWindow()
  {
    AssertTile();
    return GetTile().GetEventWindow();
  }

}
