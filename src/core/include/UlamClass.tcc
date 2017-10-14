/* -*- C++ -*- */

#include "Fail.h"
//#include "Tile.h"
#include "Random.h"
//#include "EventWindow.h"
#include "Base.h"
#include "UlamTypeInfo.h"
#include "UlamClassRegistry.h"
#include "UlamContext.h"

#include "CastOps.h" /* For _Int32ToInt32, etc */

namespace MFM {

  template <class EC>
  s32 UlamClass<EC>::PositionOfDataMember(const UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName)
  {
    const UlamElement<EC> * ueltptr = (UlamElement<EC> *) uc.LookupElementTypeFromContext(type);
    if (!ueltptr) return -2;
    s32 ret = ueltptr->PositionOfDataMemberType(dataMemberTypeName);
    if (ret < 0) return -3;
    return ret;
  } //PositionOfDataMember (static)

  template <class EC>
  bool UlamClass<EC>::IsMethod(const UlamContext<EC>& uc, u32 type, const UlamClass<EC> * classPtr)
  {
    const UlamElement<EC> * ueltptr = (UlamElement<EC> *) uc.LookupElementTypeFromContext(type);
    if (!ueltptr) return false;
    return ueltptr->internalCMethodImplementingIs(classPtr);
  } //IsMethod (static)

  typedef void (*VfuncPtr)(); // Generic function pointer we'll cast at point of use
  template <class EC>
  VfuncPtr UlamClass<EC>::GetVTableEntry(const UlamContext<EC>& uc, u32 atype, u32 idx)
  {
    if( atype == EC::ATOM_CONFIG::ATOM_TYPE::ATOM_UNDEFINED_TYPE )
      FAIL(ILLEGAL_STATE);  // needs 'quark type' vtable support
    const UlamElement<EC> * ueltptr = (UlamElement<EC> *) uc.LookupElementTypeFromContext(atype);
    if (!ueltptr) return NULL;
    return ueltptr->getVTableEntry(idx);
  } //GetVTableEntry (static)

  template <class EC>
  void UlamClass<EC>::PrintClassMembers(const UlamClassRegistry<EC> & ucr,
                                        ByteSink & bs,
                                        const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
                                        u32 flags,
                                        u32 baseStatePos,
                                        u32 indent) const
  {
    AtomBitStorage<EC> abs(atom);
    PrintClassMembers(ucr, bs, abs, flags, baseStatePos, indent);
  }

  inline static void doNL(ByteSink & bs, u32 flags, u32 indent) {
    if (flags & UlamClassPrintFlags::PRINT_INDENTED_LINES) 
    {
      bs.Printf("\n");
      for (u32 i = 0; i < indent; ++i) bs.Printf(" ");
    }
  }

  template <class EC>
  void UlamClass<EC>::PrintClassMembers(const UlamClassRegistry<EC> & ucr,
                                        ByteSink & bs,
                                        const BitStorage<EC>& stg,
                                        u32 flags,
                                        u32 baseStatePos,
                                        u32 indent) const
  {
    //    typedef typename EC::ATOM_CONFIG::ATOM_TYPE T;
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
          ++indent;
          doNL(bs,flags,indent);
        }
        else
        {
          bs.Printf(",");
          doNL(bs,flags,indent);
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
            if (utin.IsLocals()) break; // locals have no data members (and cannot be in arrays..)

            if (arraysize > 0)
            {
              if (idx>0) bs.Printf(",");
              bs.Printf("[%d]=",idx);
            }

#if 0 
            u32 offset = T::ATOM_FIRST_STATE_BIT;

            if (this->IsUlamTransient()) 
              offset = 0;
#else
            u32 offset = 0;  // barf.  barf barf barf
#endif

            u32 startPos = 
              baseStatePos + dmi.m_bitPosition
              + offset + idx * bitsize;

            if (utin.IsQuark() || utin.IsTransient())
            {
              if (flags & PRINT_RECURSE_QUARKS)
              {
                const char * mangledName = dmi.m_mangledType;
                const UlamClass * memberClass = ucr.GetUlamClassByMangledName(mangledName);
                if (memberClass)
                {
                  u32 flatFlags = flags;  
                  // Only go down one more level?
                  //flatFlags &= ~UlamClassPrintFlags::PRINT_RECURSE_QUARKS;
                  memberClass->PrintClassMembers(ucr, bs, stg, flatFlags, startPos, indent + 1);
                  continue;
                }
              }

              if (bitsize > 64) 
              {
                // Just do hex, aligned from the right
                bs.Printf("%d: 0x", bitsize); 

                u32 start = bitsize%4;
                if (start) bs.Printf("%x", stg.Read(startPos, start));

                for (u32 i = start; i < bitsize; i += 4) 
                  bs.Printf("%x", stg.Read(startPos + i, 4));

              } 
              else
              {
                u64 val = stg.ReadLong(startPos, bitsize);
                bs.Printf("0x%x", val); // Just do hex if no recursion or unknown class
              }
              continue;
            }

            if (!utin.IsPrimitive()) FAIL(ILLEGAL_STATE); // Can't happen now right?

            u64 val = stg.ReadLong(startPos, bitsize);
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

            case UlamTypeInfoPrimitive::STRING:
              {
                bs.Printf("<string>"); // XXX NYI
                break;
              }

            default:
              FAIL(ILLEGAL_STATE);
            }
          }
        }
      }
      if (opened) --indent;
      doNL(bs,flags,indent);
      if (opened) bs.Printf(")");
    }
  } //PrintClassMembers

  template <class EC>
  void UlamClass<EC>::addHex(ByteSink & bs, u64 val)
  {
    bs.Printf("/0x");
    bs.Print(val, Format::HEX);
  }

  template <class EC>
  void UlamClass<EC>::DefineRegistrationNumber(u32 num)
  {
    if (num == UNINITTED_REGISTRY_NUMBER)
      FAIL(ILLEGAL_ARGUMENT);

    if (m_ulamClassRegistryNumber != UNINITTED_REGISTRY_NUMBER)
      FAIL(DUPLICATE_ENTRY);

    m_ulamClassRegistryNumber = num;
  }

  template <class EC>
  u32 UlamClass<EC>::GetRegistrationNumber() const
  {
    if (m_ulamClassRegistryNumber == UNINITTED_REGISTRY_NUMBER)
      FAIL(ILLEGAL_STATE);

    return m_ulamClassRegistryNumber;
  }

} //MFM
