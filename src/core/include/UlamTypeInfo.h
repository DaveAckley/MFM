/*                                              -*- mode:C++ -*-
  UlamTypeInfo.h An abstract base class for ULAM info
  Copyright (C) 2015-2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015-2017 Ackleyshack LLC.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file UlamTypeInfo.h An abstract base class for ULAM info
  \author David H. Ackley.
  \author Elenas S. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \lgpl
 */
#ifndef ULAMTYPEINFO_H
#define ULAMTYPEINFO_H

#include "itype.h"
#include "Parameter.h"

namespace MFM
{

  struct UlamTypeInfoPrimitive {
    enum PrimType { VOID, INT, UNSIGNED, BOOL, UNARY, BITS, STRING };

    u8 m_primType;
    u8 m_bitSize;
    u16 m_arrayLength;

    static bool PrimTypeFromChar(const u8 ch, PrimType & result) ;

    static u8 CharFromPrimType(const PrimType type) ;

    static const char * NameFromPrimType(const PrimType type) ;

    static u32 DefaultSizeFromPrimType(const PrimType type) ;

    PrimType GetPrimType() const
    {
      return (PrimType) m_primType;
    }

    bool InitFrom(const char * mangledName)
    {
      CharBufferByteSource cbs(mangledName,strlen(mangledName));
      return InitFrom(cbs);
    }

    bool InitFrom(ByteSource & bs) ;

    void PrintMangled(ByteSink & bs) const ;
    void PrintPretty(ByteSink & bs) const ;

    u32 GetBitSize() const { return m_bitSize; }
    u32 GetArrayLength() const { return m_arrayLength; }
    void MakeScalar() { m_arrayLength = 0; }
    bool IsScalar() const { return GetArrayLength() == 0; }
    void AssertScalar() const { if (!IsScalar()) FAIL(ILLEGAL_STATE); }

    u64 GetMaxOfScalarType() const
    {
      return GetExtremeOfScalarType(true);
    }

    s64 GetMinOfScalarType() const
    {
      return (s64) GetExtremeOfScalarType(false);
    }

    u64 GetExtremeOfScalarType(bool wantMax) const ;

  };
} //MFM

namespace MFM {
  const u32 MAX_CLASS_NAME_LENGTH = 64;
  const u32 MAX_CLASS_PARAMETERS = 16;
  typedef OverflowableCharBufferByteSink<MAX_CLASS_NAME_LENGTH> OStringClassName;

  struct UlamTypeInfoParameter {
    UlamTypeInfoPrimitive m_parameterType;
    u32 m_value;  // overloaded depending on type
  };
} //MFM

namespace MFM {
  typedef UlamTypeInfoParameter UlamTypeInfoClassParameterArray[MAX_CLASS_PARAMETERS];

  struct UlamTypeInfoClass {

    OStringClassName m_name;
    u32 m_arrayLength;
    u32 m_bitSize;
    u32 m_classParameterCount;
    u32 m_modelParameterCount;
    UlamTypeInfoClassParameterArray m_classParameters;

    bool InitFrom(const char * mangledName)
    {
      CharBufferByteSource cbs(mangledName,strlen(mangledName));
      return InitFrom(cbs);
    }

    bool InitFrom(ByteSource & cbs) ;

    void PrintMangled(ByteSink & bs) const ;
    void PrintPretty(ByteSink & bs) const ;

    void MakeScalar() { m_arrayLength = 0; }

  };
} //MFM

namespace MFM {

  struct UlamTypeInfo {
    UlamTypeInfoClass m_utic;
    UlamTypeInfoPrimitive m_utip;

    enum Category { PRIM, ELEMENT, QUARK, TRANSIENT, UNKNOWN } m_category;

    UlamTypeInfo() : m_category(UNKNOWN) { }

    const UlamTypeInfoPrimitive * AsPrimitive() const 
    {
      if (!IsPrimitive()) return 0;
      return & m_utip;
    }

    bool IsPrimitive() const { return m_category == PRIM; }
    bool IsElement() const { return m_category == ELEMENT; }
    bool IsQuark() const { return m_category == QUARK; }
    bool IsTransient() const { return m_category == TRANSIENT; }
    bool IsClass() const { return IsElement() || IsQuark() || IsTransient(); }

    bool InitFrom(const char * mangledName)
    {
      CharBufferByteSource cbs(mangledName,strlen(mangledName));
      return InitFrom(cbs);
    }

    bool InitFrom(ByteSource & cbs) ;

    void PrintMangled(ByteSink & bs) const ;
    void PrintPretty(ByteSink & bs) const ;

    u32 GetBitSize() const
    {
      switch (m_category)
      {
      case PRIM: return m_utip.m_bitSize;
      case ELEMENT:
      case QUARK:
      case TRANSIENT:
        return m_utic.m_bitSize;
      default:
        FAIL(ILLEGAL_STATE);
      }
    }

    void MakeScalar() {
      switch (m_category)
      {
      case PRIM: return m_utip.MakeScalar();
      case QUARK:
      case TRANSIENT:
      case ELEMENT:
        return m_utic.MakeScalar();
      default:
        FAIL(ILLEGAL_STATE);
      }
    }

    u32 GetArrayLength() const
    {
      switch (m_category)
      {
      case PRIM: return m_utip.GetArrayLength();
      case QUARK:
      case TRANSIENT:
      case ELEMENT:
        return m_utic.m_arrayLength;
      default:
        FAIL(ILLEGAL_STATE);
      }
    }

  };

} //MFM

namespace MFM {

  template <class P>
  static P GetMinOfAs(const char * mangledType)
  {
    UlamTypeInfo uti;
    if (!uti.InitFrom(mangledType) || uti.m_category != UlamTypeInfo::PRIM)
      FAIL(ILLEGAL_ARGUMENT);
    uti.m_utip.AssertScalar();
    return (P) uti.m_utip.GetMinOfScalarType();
  }

  template <class P>
  static P GetMaxOfAs(const char * mangledType)
  {
    UlamTypeInfo uti;
    if (!uti.InitFrom(mangledType) || uti.m_category != UlamTypeInfo::PRIM)
      FAIL(ILLEGAL_ARGUMENT);
    uti.m_utip.AssertScalar();
    return (P) uti.m_utip.GetMaxOfScalarType();
  }

  template <class S>
  static s32 GetDefaulted(S * ptr, S defval)
  {
    if (!ptr) return defval;
    return *ptr;
  }

} //MFM


namespace MFM {

  /** A descriptor for a single data member in an ULAM class */
  struct UlamClassDataMemberInfo {
    const char * m_mangledType;
    const char * m_dataMemberName;
    u32 m_bitPosition;

    UlamClassDataMemberInfo(const char * mangled, const char *name, u32 pos)
      : m_mangledType(mangled)
      , m_dataMemberName(name)
      , m_bitPosition(pos)
    { }
  };

} //MFM


namespace MFM {

  template <class EC> class UlamElement; //forward

  /** MODEL PARAMETERS */
  template <class EC>
  struct UlamTypeInfoModelParameterS32 : ElementParameterS32<EC> {
    const char * GetUnits() const { return m_parameterUnits; }

    UlamTypeInfoModelParameterS32(
                                  UlamElement<EC> & theElement,
                                  const char * mangledType,
                                  const char * ulamName,
                                  const char * briefDescription,
                                  const char * details,
                                  s32 * minOrNull,
                                  s32 * defaultOrNull,
                                  s32 * maxOrNull,
                                  const char * units) ;
    const char * m_parameterUnits;
  };

} //MFM

namespace MFM{

  template <class EC> class UlamElement; //forward

  /** A descriptor for a single unsigned-value ULAM model parameter */
  template <class EC>
  struct UlamTypeInfoModelParameterU32 : ElementParameterU32<EC> {
    const char * GetUnits() const { return m_parameterUnits; }

    UlamTypeInfoModelParameterU32(
                                  UlamElement<EC> & theElement,
                                  const char * mangledType,
                                  const char * ulamName,
                                  const char * briefDescription,
                                  const char * details,
                                  u32 * minOrNull,
                                  u32 * defaultOrNull,
                                  u32 * maxOrNull,
                                  const char * units) ;
    const char * m_parameterUnits;
  };

} //MFM

namespace MFM {

  template <class EC> class UlamElement; //forward

  template <class EC>
  struct UlamTypeInfoModelParameterUnary : ElementParameterUnary<EC> {
    const char * GetUnits() const { return m_parameterUnits; }

    UlamTypeInfoModelParameterUnary(
                                  UlamElement<EC> & theElement,
                                  const char * mangledType,
                                  const char * ulamName,
                                  const char * briefDescription,
                                  const char * details,
                                  u32 * minOrNull,
                                  u32 * defaultOrNull,
                                  u32 * maxOrNull,
                                  const char * units) ;
    const char * m_parameterUnits;
  };

} //MFM

namespace MFM {

  template <class EC> class UlamElement; //forward

  template <class EC>
  struct UlamTypeInfoModelParameterBool : ElementParameterBool<EC> {

    UlamTypeInfoModelParameterBool(
				   UlamElement<EC> & theElement,
				   const char * mangledType,
				   const char * ulamName,
				   const char * briefDescription,
				   const char * details,
				   bool defvalue) ;
  };

} //MFM

#include "UlamTypeInfo.tcc"

#endif /* ULAMTYPEINFO_H */
