/* -*- C++ -*- */
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"
#include "Site.h"
//#include "UlamClass.h"
//#include "UlamElement.h"

namespace MFM {

  template <class EC> class UlamElement; //forward
  template <class EC> class UlamClass; //forward

  template <class EC>
  UlamContextEvent<EC>::UlamContextEvent(const UlamContextEvent<EC>& cxref) 
    : UlamContext<EC>(cxref.m_elementTable) 
    , m_tile(cxref.m_tile)
  { }

  template <class EC>
  void UlamContextEvent<EC>::SetTile(Tile<EC> & t)
  {
    MFM_API_ASSERT_STATE(!m_tile);
    m_tile = & t;
  }

  template <class EC>
  void UlamContextEvent<EC>::AssertTile() const
  {
    MFM_API_ASSERT_STATE(m_tile);
  }

  template <class EC>
  Tile<EC> & UlamContextEvent<EC>::GetTile()
  {
    AssertTile();
    return * m_tile;
  }

  template <class EC>
  const Tile<EC> & UlamContextEvent<EC>::GetTile() const
  {
    AssertTile();
    return * m_tile;
  }

} //MFM
