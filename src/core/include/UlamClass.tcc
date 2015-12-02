/* -*- C++ -*- */

#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"
#include "UlamTypeInfo.h"

#include "CastOps.h" /* For _Int32ToInt32, etc */

namespace MFM {

  template <class EC>
  s32 UlamClass<EC>::PositionOfDataMember(UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName)
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
  bool UlamClass<EC>::IsMethod(UlamContext<EC>& uc, u32 type, const char * quarkTypeName)
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
  VfuncPtr UlamClass<EC>::GetVTableEntry(UlamContext<EC>& uc, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom, u32 atype, u32 idx)
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

  template <class EC>
  void UlamClass<EC>::PrintClassMembers(const UlamClassRegistry<EC> & ucr,
                                    ByteSink & bs,
                                    const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
                                    u32 flags,
                                    u32 baseStatePos) const
  {
    typedef typename EC::ATOM_CONFIG::ATOM_TYPE T;
    if (flags & (PRINT_MEMBER_VALUES|PRINT_MEMBER_NAMES|PRINT_MEMBER_TYPES))
    {
      bool opened = false;
      for (s32 i = 0; i < GetDataMemberCount(); ++i)
      {
        const UlamClassDataMemberInfo & dmi = GetDataMemberInfo((u32) i);
	UlamTypeInfo utin;
        if (!utin.InitFrom(dmi.m_mangledType))
          FAIL(ILLEGAL_STATE);

        // Skip size 0 members unless they reeeally want them
        if (utin.GetBitSize() == 0 && !(flags & PRINT_SIZE0_MEMBERS)) continue;

        if (!opened)
        {
          opened = true;
          bs.Printf("(");
        }
        else
        {
          bs.Printf(",");
        }
        if (flags & PRINT_MEMBER_TYPES)
        {
          utin.PrintPretty(bs);
          bs.Printf(" ");
        }

        if (flags & PRINT_MEMBER_NAMES)
        {
          bs.Printf("%s", dmi.m_dataMemberName);
          if (flags & PRINT_MEMBER_VALUES)
            bs.Printf("=");
        }

        if (flags & PRINT_MEMBER_VALUES)
        {
          // For starters just dig out the bits and print them
          u32 bitsize = utin.GetBitSize();
          u32 arraysize = utin.GetArrayLength();

          for (u32 idx = 0; idx < MAX(arraysize,1u); ++idx)
          {
            if (arraysize > 0)
            {
              if (idx==0) bs.Printf("[%d]",arraysize);
              bs.Printf(", [%d]=",idx);
            }

            u64 val = atom.GetBits().ReadLong(baseStatePos + dmi.m_bitPosition
                                              + T::ATOM_FIRST_STATE_BIT
                                              + idx * bitsize,
                                              bitsize);

            if (utin.m_category == UlamTypeInfo::QUARK)
            {
              if (flags & PRINT_RECURSE_QUARKS)
              {
                const char * mangledName = dmi.m_mangledType;
                const UlamClass * memberClass = ucr.GetUlamClassByMangledName(mangledName);
                if (memberClass)
                {
                  memberClass->PrintClassMembers(ucr, bs, atom, flags, baseStatePos + dmi.m_bitPosition);
                  continue;
                }
              }
              bs.Printf("0x%x", val); // Just do hex if no recursion or unknown class
              continue;
            }

            if (utin.m_category != UlamTypeInfo::PRIM) FAIL(ILLEGAL_STATE); // Can't happen now right?

            switch (utin.m_utip.GetPrimType())
            {
            case UlamTypeInfoPrimitive::INT:
              {
                s64 cval = _SignExtend64(val,bitsize);
                bs.Print(cval);
                if (flags & PRINT_MEMBER_BITVALS) addHex(bs,val);
                break;
              }

            case UlamTypeInfoPrimitive::UNSIGNED:
              {
                bs.Print(val);
                if (flags & PRINT_MEMBER_BITVALS) addHex(bs,val);
                break;
              }

            case UlamTypeInfoPrimitive::BOOL:
              {
                bool cval = _Bool64ToCbool(val,bitsize);
                bs.Printf("%s", cval?"true":"false");
                if (flags & PRINT_MEMBER_BITVALS) addHex(bs, val);
                break;
              }

            case UlamTypeInfoPrimitive::UNARY:
              {
                u32 cval = (u32) _Unary64ToInt64(val,bitsize,32);
                bs.Printf("%d", cval);
                if (flags & PRINT_MEMBER_BITVALS) addHex(bs, val);
                break;
              }

            case UlamTypeInfoPrimitive::BITS:
              {
                bs.Printf("0x");  // use hex for bits
                bs.Print(val, Format::HEX);
                if (flags & PRINT_MEMBER_BITVALS)  // and binary for 'raw'
                {
                  bs.Printf("/");
                  bs.Print(val, Format::BIN, bitsize, '0');
                }
                break;
              }

            case UlamTypeInfoPrimitive::VOID:
              {
                bs.Printf("void"); // should be impossible?
                break;
              }
            default:
              FAIL(ILLEGAL_STATE);
            }
          }
        }
      }
      if (opened)
        bs.Printf(")");
    }
  } //PrintClassMembers

  template <class EC>
  void UlamClass<EC>::addHex(ByteSink & bs, u64 val)
  {
    bs.Printf("/0x");
    bs.Print(val, Format::HEX);
  }

} //MFM
