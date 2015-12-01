/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"

#include "CastOps.h" /* For _Int32ToInt32, etc */
#include "UlamTypeInfo.h"

namespace MFM {

  template <class EC>
  s32 UlamClassTemplated<EC>::PositionOfDataMember(UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName)
  {
    Tile<EC> & tile = uc.GetTile();
    ElementTable<EC> & et = tile.GetElementTable();
    const Element<EC> * eltptr = et.Lookup(type);
    if (!eltptr) return -1;
    const UlamElement<EC> * ueltptr = eltptr->AsUlamElement();
    if (!ueltptr) return -2;
    s32 ret = ueltptr->PositionOfDataMemberType(dataMemberTypeName);
    if (ret < 0) return -3;
    return ret;
  } //PositionOfDataMember (static)

  template <class EC>
  bool UlamClassTemplated<EC>::IsMethod(UlamContext<EC>& uc, u32 type, const char * quarkTypeName)
  {
    Tile<EC> & tile = uc.GetTile();
    ElementTable<EC> & et = tile.GetElementTable();
    const Element<EC> * eltptr = et.Lookup(type);
    if (!eltptr) return false;
    const UlamElement<EC> * ueltptr = eltptr->AsUlamElement();
    if (!ueltptr) return false;
    return ueltptr->internalCMethodImplementingIs(quarkTypeName);
  } //IsMethod (static)

  typedef void (*VfuncPtr)(); // Generic function pointer we'll cast at point of use
  template <class EC>
  VfuncPtr UlamClassTemplated<EC>::GetVTableEntry(UlamContext<EC>& uc, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom, u32 atype, u32 idx)
  {
    if( atype == EC::ATOM_CONFIG::ATOM_TYPE::ATOM_UNDEFINED_TYPE )
      FAIL(ILLEGAL_STATE);  // needs 'quark type' vtable support

    Tile<EC> & tile = uc.GetTile();
    ElementTable<EC> & et = tile.GetElementTable();
    const Element<EC> * eltptr = et.Lookup(atype);
    if (!eltptr) return NULL;
    const UlamElement<EC> * ueltptr = eltptr->AsUlamElement();
    if (!ueltptr) return NULL;
    return ueltptr->getVTableEntry(idx);
  } //GetVTableEntry (static)

} //MFM
