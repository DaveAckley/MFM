/* -*- C++ -*- */
#include "Fail.h"
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
  UlamContext<EC>::UlamContext() : m_tile(0) { }

  template <class EC>
  UlamContext<EC>::UlamContext(const UlamContext<EC>& cxref) : m_tile(cxref.m_tile) { }

  template <class EC>
  void UlamContext<EC>::SetTile(Tile<EC> & t)
  {
    MFM_API_ASSERT_STATE(!m_tile);
    m_tile = & t;
  }

  template <class EC>
  void UlamContext<EC>::AssertTile() const
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
  const Tile<EC> & UlamContext<EC>::GetTile() const
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
  const EventWindow<EC> & UlamContext<EC>::GetEventWindow() const
  {
    AssertTile();
    return GetTile().GetEventWindow();
  }

  template <class EC>
  EventWindow<EC> & UlamContext<EC>::GetEventWindow()
  {
    AssertTile();
    return GetTile().GetEventWindow();
  }

  template <class EC>
  Base<typename EC::ATOM_CONFIG> & UlamContext<EC>::GetBase()
  {
    return GetEventWindow().GetBase();
  }

  template <class EC>
  const Site<typename EC::ATOM_CONFIG> & UlamContext<EC>::GetSite() const
  {
    return GetEventWindow().GetSite();
  }

  template <class EC>
  const UlamClass<EC> * UlamContext<EC>::LookupElementTypeFromContext(u32 etype) const
  {
    const Tile<EC> & tile = GetTile();
    const ElementTable<EC> & et = tile.GetElementTable();
    const Element<EC> * eltptr = et.Lookup(etype);
    if (!eltptr) return NULL;
    const UlamElement<EC> * ueltptr =  eltptr->AsUlamElement();
    return ueltptr; //might be NULL
  } //LookupElementTypeFromContext

} //MFM
