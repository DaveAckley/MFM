/*                                              -*- mode:C++ -*-
  UlamElement.h A concrete base class for ULAM elements
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \file UlamElement.h A concrete base class for ULAM elements
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ULAMELEMENT_H
#define ULAMELEMENT_H

#include "Element.h"
#include "Tile.h"
#include "EventWindow.h"
#include "UlamContext.h"

// Unsigned(32)
#ifndef Ud_Ui_Ut_102321u
#define Ud_Ui_Ut_102321u
namespace MFM{
  struct Ui_Ut_102321u
  {
    typedef BitField<BitVector<32>, VD::U32, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102321u() : m_stg() { }
    Ui_Ut_102321u(const u32 d) : m_stg(d) {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102321u */

// Unsigned(8) [4] -- for ARGB colors
#ifndef Ud_Ui_Ut_14181u
#define Ud_Ui_Ut_14181u
namespace MFM{
  struct Ui_Ut_14181u
  {
    typedef BitField<BitVector<32>, VD::BITS, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_14181u() : m_stg() { }
    Ui_Ut_14181u(const u32 d) : m_stg(d) {}
    const u32 read() const { return BF::Read(m_stg); }   //reads entire array
    const u32 readArrayItem(const u32 index, const u32 unitsize) const { return BF::ReadArray(m_stg, index, unitsize); }
    void write(const u32 v) { BF::Write(m_stg, v); }   //writes entire array
    void writeArrayItem(const u32 v, const u32 index, const u32 unitsize) { BF::WriteArray(m_stg, v, index, unitsize); }
  };
} //MFM
#endif /*Ud_Ui_Ut_14181u */

namespace MFM
{

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
  typedef OverflowableCharBufferByteSink<MAX_CLASS_NAME_LENGTH> OStringClassName;

  struct UlamTypeInfoClassParameter {
    UlamTypeInfoPrimitive m_classParameterType;
    u32 m_value;  // overloaded depending on type
  };

  typedef UlamTypeInfoClassParameter UlamTypeInfoClassParameterArray[MAX_CLASS_PARAMETERS];

  struct UlamTypeInfoClass {

    OStringClassName m_name;
    u32 m_arrayLength;
    u32 m_bitSize;
    u32 m_parameterCount;
    UlamTypeInfoClassParameterArray m_classParameters;

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

  struct UlamClass {
    /**
       Specify the number of data members in this class.  To be
       overridden by subclasses of UlamClassInfo.

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

  };
}

namespace MFM
{
  template <class EC> class UlamElement; // FORWARD

  template <class EC>
  struct UlamElementInfo
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    virtual const char * GetName() const = 0;
    virtual const char * GetSymbol() const = 0;
    virtual const char * GetSummary() const = 0;
    virtual const char * GetDetails() const = 0;
    virtual const char * GetAuthor() const = 0;
    virtual const char * GetLicense() const = 0;
    virtual bool GetPlaceable() const = 0;
    virtual const u32 GetVersion() const = 0;
    virtual const u32 GetNumColors() const = 0;
    virtual const u32 GetElementColor() const = 0;
    virtual const u32 GetSymmetry(UlamContext<EC>& uc) const = 0;

    virtual const u32 GetPercentDiffusability() const
    {
      return 100;
    }

    UlamElementInfo() { }
    virtual ~UlamElementInfo() { }
  };

  /**
   * A UlamElement is a concrete element primarily for use by culam.
   */
  template <class EC>
  class UlamElement : public Element<EC>, public UlamClass
  {
    typedef Element<EC> Super;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    const UlamElementInfo<EC> * m_info;

  public:

    UlamElement(const UUID & uuid) : Element<EC>(uuid)
    { }

    /**
       Flag values determining what Print(Atom,u32) prints
       \sa Print(ByteSink&, const T&, u32)
     */
    enum PrintFlags {
      PRINT_SYMBOL =          0x00000001, //< Include element symbol
      PRINT_FULL_NAME =       0x00000002, //< Include element name
      PRINT_ATOM_BODY =       0x00000004, //< Include entire atom in hex
      PRINT_MEMBER_VALUES =   0x00000008, //< Include data member values
      PRINT_MEMBER_NAMES =    0x00000010, //< Include data member names
      PRINT_MEMBER_TYPES =    0x00000020, //< Include data member types
      PRINT_SIZE0_MEMBERS =   0x00000040, //< Include size 0 data members
      PRINT_MEMBER_ARRAYS =   0x00000080, //< Print array values individually
      PRINT_RECURSE_QUARKS =  0x00000100, //< Print quarks recursively

      /** (Composite value) Print nothing */
      PRINT_NOTHING = 0,

      /** (Composite value) Print element symbol and entire atom in hex */
      PRINT_HEX_ATOM = PRINT_SYMBOL|PRINT_ATOM_BODY,

      /** (Composite value) Print element symbol and its data member values in declaration order */
      PRINT_TOP_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_VALUES,

      /** (Composite value) Print element symbol and its data member names and values in declaration order */
      PRINT_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_NAMES|PRINT_MEMBER_VALUES,

      /** (Composite value) Print element symbol and data member values, expanding quarks */
      PRINT_QUARK_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_VALUES|PRINT_RECURSE_QUARKS,

      /** (Composite value) Print element symbol and data member values, expanding quarks and arrays */
      PRINT_ALL_MEMBERS = PRINT_SYMBOL|PRINT_MEMBER_VALUES|PRINT_RECURSE_QUARKS|PRINT_MEMBER_ARRAYS,

      /** (Composite value) Print far too much */
      PRINT_EVERYTHING = -1

    };
    /**
       Print the contents of atom to the given ByteSink, including
       various details as specified by flags.
       \sa PrintFlags
     */
    void Print(ByteSink & bs, const T & atom, u32 flags) const ;

    void SetInfo(const UlamElementInfo<EC> * info) {
      m_info = info;
      this->SetName(m_info->GetName());
      this->SetAtomicSymbol(m_info->GetSymbol());
    }

    /**
     * Destroys this UlamElement.
     */
    virtual ~UlamElement()
    { }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      Tile<EC> & tile = window.GetTile();
      UlamContext<EC> uc;
      uc.SetTile(tile);

      u32 sym = m_info ? m_info->GetSymmetry(uc) : PSYM_DEG000L;
      window.SetSymmetry((PointSymmetry) sym);

      T & me = window.GetCenterAtomSym();
      Uf_6behave(uc, me);
    }

    /**
       Ulam elements that define 'Void behave()' will override this
       method, and it will be called on events!
     */
    virtual void Uf_6behave(UlamContext<EC> & uc, T& self) const
    {
      // Empty by default
    }

    /**
       Ulam elements defining 'Unsigned getColor(Unsigned selector)'
       will override this method, and it will be called during
       graphics rendering!

       Note the Uv_4self in this method IS A COPY of the atom being
       rendered -- any changes made to Uv_4self during this method
       will vanish when it returns.

       Note also that THERE IS NO EVENT IN PROGRESS when this method
       is called!  Any attempt to access event services during this
       method will fail!  That includes event window accesses AND
       random numbers!
     */
    virtual Ui_Ut_14181u Uf_8getColor(UlamContext<EC>& uc,
                                             T& Uv_4self,
                                             Ui_Ut_102321u Uv_8selector) const
    {
      return Ui_Ut_14181u(0xffffffff);
    }

    virtual bool GetPlaceable() const
    {
      return m_info?m_info->GetPlaceable() : true;
    }

    virtual UlamElement* AsUlamElement()
    {
      return this;
    }

    virtual const UlamElement<EC> * AsUlamElement() const
    {
      return this;
    }

    /**
       Find the first position of a data member in this element by the
       name of its type, if any exist.

       \return The smallest bit position of an occurrence of the type
               \c dataMemberTypeName in this element, if any, or -1 to
               indicate the given type is not used as a data member in
               this UlamElement.  A return value of 0 corresponds to
               the ATOM_FIRST_STATE_BIT of the atom.

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual s32 PositionOfDataMemberType(const char * dataMemberTypeName) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Find the first position of a data member, specified by its \c
       dataMemberTypeName, in an UlamElement specified by its \c type
       number, if such as UlamElement exists and has such a data
       member.

       \param type an element type number, hopefully of an UlamElement

       \param dataMemberTypeName the name of the type to search for in
              the data members of the found UlamElement.

       \return The smallest bit position of an occurrence of the type
               \c dataMemberTypeName in this element, if any.  Note
               that a return value of 0 corresponds to the
               ATOM_FIRST_STATE_BIT of the atom.

               A return value of -1 indicates the given \c type is not
               associated with any known type of element.

               A return value of -2 indicates the given \c type is
               associated with an Element that is not an UlamElement,
               so its data members cannot be searched.

               A return value of -3 indicates the given \c type is
               associated with an UlamElement, which did not contain a
               data member of the named type.

       \sa T::ATOM_FIRST_STATE_BIT
       \sa PositionOfDataMemberType
     */
    static s32 PositionOfDataMember(UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName)
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
    }

    virtual u32 DefaultPhysicsColor() const
    {
      if (m_info) {
        return m_info->GetElementColor();
      }
      return 0xffffffff;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      /* XXX REWRITE!
      if (m_info && m_info->GetNumColors() > selector) {
        UlamContext<EC> uc;
        return m_info->GetColor(uc, atom, selector);
      }
      */
      return this->PhysicsColor();
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      if (nowAt == maybeAt || !m_info) return COMPLETE_DIFFUSABILITY;
      return
        COMPLETE_DIFFUSABILITY * m_info->GetPercentDiffusability() / 100;
    }
  };
} // MFM

#include "UlamElement.tcc"

#endif /* ULAMELEMENT_H */
