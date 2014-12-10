/* -*- C++ -*- */
#include "Fail.h"

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
    return THE_INSTANCE;
  }

  template <class CC> UlamContext<CC> UlamContext<CC>::THE_INSTANCE;
}
