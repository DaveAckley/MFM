/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"

namespace MFM {

  template <class CC>
  UlamContext<CC>::UlamContext() : m_tile(0) { }

  template <class CC>
  void UlamContext<CC>::SetTile(Tile<CC> & t)
  {
    m_tile = & t;
  }

  template <class CC>
  void UlamContext<CC>::AssertTile()
  {
    if (!m_tile) FAIL(ILLEGAL_STATE);
  }

  template <class CC>
  Tile<CC> & UlamContext<CC>::GetTile()
  {
    AssertTile();
    return * m_tile;
  }

  template <class CC>
  Random & UlamContext<CC>::GetRandom()
  {
    AssertTile();
    return GetTile().GetRandom();
  }

  template <class CC>
  EventWindow<CC> & UlamContext<CC>::GetEventWindow()
  {
    AssertTile();
    return GetTile().GetEventWindow();
  }

  template <class CC>
  UlamContext<CC> & UlamContext<CC>::Get() {
    // an UlamContext is (way) mutable!  It must be per thread!
    static __thread UlamContext<CC> THE_INSTANCE;
    return THE_INSTANCE;
  }
}
