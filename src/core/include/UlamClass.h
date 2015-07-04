/*                                              -*- mode:C++ -*-
  UlamClass.h An abstract base class for ULAM quarks and elements
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file UlamClass.h An abstract base class for ULAM quarks and elements
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef ULAMCLASS_H
#define ULAMCLASS_H

#include "Element.h"

namespace MFM
{
  struct UlamClass; // FORWARD

  struct UlamTypeInfoPrimitive {
    enum PrimType { VOID, INT, UNSIGNED, BOOL, UNARY, BITS };

    static bool PrimTypeFromChar(const u8 ch, PrimType & result) ;

    static u8 CharFromPrimType(const PrimType type) ;

    static const char * NameFromPrimType(const PrimType type) ;

    static u32 DefaultSizeFromPrimType(const PrimType type) ;

    u8 m_primType;
    u8 m_bitSize;
    u16 m_arrayLength;

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
  };

  const u32 MAX_CLASS_NAME_LENGTH = 64;
  const u32 MAX_CLASS_PARAMETERS = 16;
  const u32 MAX_MODEL_PARAMETERS = 16;
  typedef OverflowableCharBufferByteSink<MAX_CLASS_NAME_LENGTH> OStringClassName;

  struct UlamTypeInfoParameter {
    UlamTypeInfoPrimitive m_parameterType;
    u32 m_value;  // overloaded depending on type
  };

  struct UlamTypeInfoModelParameter  {
    const char * m_parameterName;
    const char * m_parameterDoc;
    UlamTypeInfoParameter m_typeAndValue;
    u32 m_min;      // overloaded depending on type
    u32 m_default;  // overloaded depending on type
    u32 m_max;      // overloaded depending on type
  };

  typedef UlamTypeInfoParameter UlamTypeInfoClassParameterArray[MAX_CLASS_PARAMETERS];

  typedef UlamTypeInfoModelParameter UlamTypeInfoModelParameterArray[MAX_MODEL_PARAMETERS];

  struct UlamTypeInfoClass {

    OStringClassName m_name;
    u32 m_arrayLength;
    u32 m_bitSize;
    u32 m_classParameterCount;
    u32 m_modelParameterCount;
    UlamTypeInfoClassParameterArray m_classParameters;
    UlamTypeInfoModelParameterArray m_modelParameters;

    bool InitFrom(const char * mangledName)
    {
      CharBufferByteSource cbs(mangledName,strlen(mangledName));
      return InitFrom(cbs);
    }

    bool InitFrom(ByteSource & cbs) ;

    void PrintMangled(ByteSink & bs) const ;
    void PrintPretty(ByteSink & bs) const ;
  };

  struct UlamTypeInfo {
    UlamTypeInfoClass m_utic;
    UlamTypeInfoPrimitive m_utip;

    enum Category { PRIM, ELEMENT, QUARK, UNKNOWN } m_category;

    UlamTypeInfo() : m_category(UNKNOWN) { }

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
        return m_utic.m_bitSize;
      default:
        FAIL(ILLEGAL_STATE);
      }
    }

  };

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

  struct UlamClassRegistry {
    enum {
      TABLE_SIZE = 100
    };

    bool RegisterUlamClass(UlamClass& uc) ;

    s32 GetUlamClassIndex(const char *) const;

    bool IsRegisteredUlamClass(const char *mangledName) const
    {
      return GetUlamClassIndex(mangledName) >= 0;
    }

    const UlamClass* GetUlamClassByMangledName(const char *mangledName) const
    {
      s32 idx = GetUlamClassIndex(mangledName);
      if (idx < 0) return 0;
      return GetUlamClassByIndex((u32) idx);
    }

    const UlamClass* GetUlamClassByIndex(u32 index) const
    {
      if (index >= m_registeredUlamClassCount) FAIL(ILLEGAL_ARGUMENT);
      return m_registeredUlamClasses[index];
    }

    UlamClass * m_registeredUlamClasses[TABLE_SIZE];
    u32 m_registeredUlamClassCount;

  };

  struct UlamClass {

    void AddModelParameter(const char * mangledName, BitVector<32> & current, u32 min, u32 def, u32 max)
    {
      FAIL(INCOMPLETE_CODE);
    }

    /**
       Specify the mangled name of this class.  To be
       overridden by subclasses of UlamClass.

       \return a pointer to a statically-allocated const char *.
       Never returns NULL.

     */
    virtual const char * GetMangledClassName() const
    = 0;

    /**
       Specify the number of data members in this class.  To be
       overridden by subclasses of UlamClass.

       \return -1 means the data members are unknown

       \return 0 means no data members, so GetDataMemberInfo should
       not be called.

     */
    virtual s32 GetDataMemberCount() const
    {
      return -1;
    }

    /**
       Gain access to the info about a specific data member in this
       class.  To be overridden by subclasses of UlamClassInfo.
     */
    virtual const UlamClassDataMemberInfo & GetDataMemberInfo(u32 dataMemberNumber) const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
       Flag values determining what Print(Atom,u32) prints
       \sa Print(ByteSink&, const T&, u32)
     */
    enum PrintFlags {
      PRINT_SYMBOL =          0x00000001, //< Include element symbol
      PRINT_FULL_NAME =       0x00000002, //< Include element name
      PRINT_ATOM_BODY =       0x00000004, //< Include entire atom in hex
      PRINT_MEMBER_VALUES =   0x00000008, //< Include data member values
      PRINT_MEMBER_BITVALS =  0x00000010, //< Include data member values as bits in hex
      PRINT_MEMBER_NAMES =    0x00000020, //< Include data member names
      PRINT_MEMBER_TYPES =    0x00000040, //< Include data member types
      PRINT_SIZE0_MEMBERS =   0x00000080, //< Include size 0 data members
      PRINT_MEMBER_ARRAYS =   0x00000100, //< Print array values individually
      PRINT_RECURSE_QUARKS =  0x00000200, //< Print quarks recursively

      /** (Composite value) Print element symbol and entire atom in hex */
      PRINT_HEX_ATOM = PRINT_SYMBOL|PRINT_ATOM_BODY,

      /** (Composite value) Print element symbol and its data member values in declaration order */
      PRINT_TOP_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_VALUES,

      /** (Composite value) Print element symbol and its data member names and values in declaration order */
      PRINT_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_NAMES|PRINT_MEMBER_VALUES,

      /** (Composite value) Print element symbol and data member values, expanding quarks */
      PRINT_QUARK_MEMBERS = PRINT_MEMBERS|PRINT_RECURSE_QUARKS,

      /** (Composite value) Print element symbol and data member values, expanding quarks and arrays */
      PRINT_ALL_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_VALUES|PRINT_RECURSE_QUARKS|PRINT_MEMBER_ARRAYS,

      /** (Composite value) Print far too much */
      PRINT_EVERYTHING = -1

    };

    template <class EC>
    void PrintClassMembers(const UlamClassRegistry & ucr,
                           ByteSink & bs,
                           const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
                           u32 flags,
                           u32 baseStatePos) const ;

  };

} // MFM

#include "UlamClass.tcc"

#endif /* ULAMCLASS_H */
