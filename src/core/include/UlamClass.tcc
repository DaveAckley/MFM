/* -*- C++ -*- */

#include "CastOps.h" /* For _Int32ToInt32, etc */

namespace MFM {
  template <class EC>
  UlamTypeInfoModelParameterS32<EC>::UlamTypeInfoModelParameterS32(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  s32 * minOrNull,
                  s32 * defaultOrNull,
                  s32 * maxOrNull,
                  const char * units)
  : ElementParameterS32<EC>(&theElement, ulamName, briefDescription, details,
                            GetDefaulted(minOrNull, GetMinOfAs<s32>(mangledType)),
                            GetDefaulted(defaultOrNull, 0),
                            GetDefaulted(maxOrNull, GetMaxOfAs<s32>(mangledType)))
    , m_parameterUnits(units)
  { }

  template <class EC>
  UlamTypeInfoModelParameterU32<EC>::UlamTypeInfoModelParameterU32(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  u32 * minOrNull,
                  u32 * defaultOrNull,
                  u32 * maxOrNull,
                  const char * units)
  : ElementParameterU32<EC>(&theElement, ulamName, briefDescription, details,
                            GetDefaulted(minOrNull, GetMinOfAs<u32>(mangledType)),
                            GetDefaulted(defaultOrNull, 0u),
                            GetDefaulted(maxOrNull, GetMaxOfAs<u32>(mangledType)))
    , m_parameterUnits(units)
  { }


  template <class EC>
  UlamTypeInfoModelParameterUnary<EC>::UlamTypeInfoModelParameterUnary(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  u32 * minOrNull,
                  u32 * defaultOrNull,
                  u32 * maxOrNull,
                  const char * units)
  : ElementParameterUnary<EC>(&theElement, ulamName, briefDescription, details,
                              GetDefaulted(minOrNull, GetMinOfAs<u32>(mangledType)),
                              GetDefaulted(defaultOrNull, 0u),
                              GetDefaulted(maxOrNull, GetMaxOfAs<u32>(mangledType)))
    , m_parameterUnits(units)
  { }

  template <class EC>
  UlamTypeInfoModelParameterBool<EC>::UlamTypeInfoModelParameterBool(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  bool defvalue)
    : ElementParameterBool<EC>(&theElement, ulamName, briefDescription, details, defvalue)
  { }

  template <class EC>
  void UlamClass::PrintClassMembers(const UlamClassRegistry & ucr,
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
          u32 val = atom.GetBits().Read(baseStatePos + dmi.m_bitPosition + T::ATOM_FIRST_STATE_BIT, bitsize);

          if (utin.m_category == UlamTypeInfo::QUARK)
          {
            if (flags & PRINT_RECURSE_QUARKS)
            {
              const char * mangledName = dmi.m_mangledType;
              const UlamClass * memberClass = ucr.GetUlamClassByMangledName(mangledName);
              if (memberClass)
              {
                memberClass->PrintClassMembers<EC>(ucr, bs, atom, flags, baseStatePos + dmi.m_bitPosition);
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
              s32 cval = _SignExtend32(val,bitsize);
              bs.Printf("%d", cval);
              if (flags & PRINT_MEMBER_BITVALS) bs.Printf("/0x%x", val);
              break;
            }

          case UlamTypeInfoPrimitive::UNSIGNED:
            {
              bs.Printf("%u", val);
              if (flags & PRINT_MEMBER_BITVALS) bs.Printf("/0x%x", val);
              break;
            }

          case UlamTypeInfoPrimitive::BOOL:
            {
              bool cval = _Bool32ToCbool(val,bitsize);
              bs.Printf("%s", cval?"true":"false");
              if (flags & PRINT_MEMBER_BITVALS) bs.Printf("/0x%x", val);
              break;
            }

          case UlamTypeInfoPrimitive::UNARY:
            {
              s32 cval = _Unary32ToInt32(val,bitsize,32);
              bs.Printf("%d", cval);
              if (flags & PRINT_MEMBER_BITVALS) bs.Printf("/0x%x", val);
              break;
            }

          case UlamTypeInfoPrimitive::BITS:
            {
              bs.Printf("0x%x", val);  // use hex for bits
              if (flags & PRINT_MEMBER_BITVALS)  // and binary for 'raw'
              {
                bs.Printf("/");
                char fmt[6] = "%000b";
                if (bitsize > 9) fmt[2] = '0'+bitsize/10;
                fmt[3] = '0'+bitsize%10;
                bs.Printf(fmt, val);
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
      if (opened)
        bs.Printf(")");
    }
  }
}
