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
    if (m_tile) FAIL(ILLEGAL_STATE);
    m_tile = & t;
  }

  template <class EC>
  void UlamContext<EC>::AssertTile()
  {
    if (!m_tile) FAIL(ILLEGAL_STATE);
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
