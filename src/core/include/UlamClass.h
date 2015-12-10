/*                                              -*- mode:C++ -*-
  UlamClass.h An abstract base class for ULAM quarks and elements
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015 Ackleyshack LLC.

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
  \author Elenas S. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */

#ifndef ULAMCLASS_H
#define ULAMCLASS_H

#include "itype.h"
#include "Element.h"
#include "UlamContext.h"
#include "UlamClassRegistry.h"

#ifndef Ud_AUTOREFBASE
#define Ud_AUTOREFBASE
namespace MFM{

  template<class EC>
  struct AutoRefBase
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    T& m_stgToChange;  //ref to storage here!
    const u32 m_pos;   //pos in atom

    AutoRefBase(T& targ, u32 idx) : m_stgToChange(targ), m_pos(idx) { }
    AutoRefBase(AutoRefBase<EC>& arg, u32 idx) : m_stgToChange(arg.getRef()), m_pos(arg.getPosOffset() + idx) { }
    ~AutoRefBase( ){ }

    const u32 read(u32 len) const { return m_stgToChange.GetBits().Read(m_pos + T::ATOM_FIRST_STATE_BIT, len); }
    const T read() const { return m_stgToChange; } //entire atom
    const u32 readArrayItem(u32 index, u32 itemlen) { return m_stgToChange.GetBits().Read((index * itemlen) + m_pos + T::ATOM_FIRST_STATE_BIT, itemlen); }
    void write(const u32 v, u32 len) { m_stgToChange.GetBits().Write(m_pos + T::ATOM_FIRST_STATE_BIT, len, v);}
    void write(const T& t) { m_stgToChange = t; } //entire atom
    void writeArrayItem(const u32 v, u32 index, u32 itemlen) { m_stgToChange.GetBits().Write((index * itemlen) + m_pos + T::ATOM_FIRST_STATE_BIT, itemlen, v); }
    BitVector<BPA>& getBits() { return m_stgToChange.GetBits(); }
    const BitVector<BPA>& getBits() const { return m_stgToChange.GetBits(); }
    T& getRef() { return m_stgToChange; }
    u32 getType() { return m_stgToChange.GetType(); }
    const u32 getPosOffset() const { return m_pos; }
  };
} //MFM
#endif /*Ud_AUTOREFBASE */

namespace MFM
{

  struct UlamClassDataMemberInfo; //forward

  template <class EC>
  struct UlamClass
  {
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
    static s32 PositionOfDataMember(const UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName) ;


    /**
       Find the ancestor of quark in this element's family tree by the
       name of its type, if any exist.

       \return true if they are related;

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual bool internalCMethodImplementingIs(const char * quarkTypeName) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Compare type of this element to atom

       \return true if they are related;

       \sa T::ATOM_FIRST_STATE_BIT
     */
    bool internalCMethodImplementingIs(const typename EC::ATOM_CONFIG::ATOM_TYPE& targ) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
      //return (this->GetType() == targ.GetType());
    }

    /**
       Discover if quark Type, specified by its \c
       quarkTypeName, in an UlamElement specified by its \c type
       number, if such as UlamElement exists and inherits from such a quark.

       \param type an element type number, hopefully of an UlamElement

       \param quarkTypeName the name of the type to search for in
              the ancestors of the found UlamElement.

       \return A return value of true indicates the given \c type is
               related to type of quark.

       \sa T::ATOM_FIRST_STATE_BIT
       \sa internalCMethodImplementingIs
     */
    static bool IsMethod(const UlamContext<EC>& uc, u32 type, const char * quarkTypeName);

    typedef void (*VfuncPtr)(); // Generic function pointer we'll cast at point of use
    /**
       Return vtable of this element, or NULL if there isn't one.

       \return ptr to function pointers
     */
    virtual VfuncPtr getVTableEntry(u32 idx) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
      return (VfuncPtr) NULL;
    }

    static VfuncPtr GetVTableEntry(const UlamContext<EC>& uc, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom, u32 atype, u32 idx);


    /**
       Specify the mangled name of this class.  To be
       overridden by subclasses of UlamClass.

       \return a pointer to a statically-allocated const char *.
       Never returns NULL.

     */
    virtual const char * GetMangledClassName() const = 0;

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

    void PrintClassMembers(const UlamClassRegistry<EC> & ucr,
			   ByteSink & bs,
			   const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
			   u32 flags,
			   u32 baseStatePos) const ;

    static void addHex(ByteSink & bs, u64 val) ;
  };

} // MFM

#include "UlamClass.tcc"

#endif /* ULAMCLASS_H */
