/*                                              -*- mode:C++ -*-
  UlamClass.h An abstract base class for ULAM quarks and elements
  Copyright (C) 2015-2019 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015-2019 Ackleyshack LLC.

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
  \author Elena S. Ackley.
  \date (C) 2015-2019 All rights reserved.
  \lgpl
 */

#ifndef ULAMCLASS_H
#define ULAMCLASS_H

#include "itype.h"
#include "BitStorage.h"
#include "UlamVTableEntry.h"

namespace MFM
{

  template <class EC> class Element; // FORWARD
  template <class EC> class UlamElement; // FORWARD
  template <class EC> struct UlamContext; // FORWARD
  template <class EC> struct UlamClassRegistry; // FORWARD
  template <class EC> struct UlamQuark; // FORWARD
  template <class EC> struct UlamTransient; // FORWARD

  struct UlamClassDataMemberInfo; //forward

  /**
     Wrapper class to provide the UlamClass::PrintClassMember flags
     outside of a template without polluting the MFM namespace
     directly.
  */
  struct UlamClassPrintFlags {
    /**
       Flag values determining what Print(Atom,u32) prints
       \sa Print(ByteSink&, const T&, u32)
    */
    enum {
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
      PRINT_INDENTED_LINES =  0x00000400, //< Add newlines and indents
      PRINT_MEMBER_ASCII =    0x00000800, //< Add ASCII data member values if printable
      PRINT_FORMAT_JSON =     0x00001000, //< Format data members in JSON

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
  };


  template <class EC>
  struct UlamClass : UlamClassPrintFlags
  {
    /**
       Stub for the class-specific ulam string lookup function.
       \c index the string index into the class user string pool.
     */
    virtual const u8 * GetString(u32 index) const
    {
      FAIL(ILLEGAL_STATE);
    }

    virtual u32 GetStringLength(u32 index) const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
	Returns the bit size of this class as a complete object,
	including all baseclasses.
    */
    virtual u32 GetClassLength() const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
	Returns the bit size of this class' data members,
	excluding baseclasses; this is its baseclass
	size, shared; Elements cannot be shared, or bases.
    */
    virtual u32 GetClassDataMembersSize() const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
	Returns the number of base classes + self, two minimum: one
	for UrSelf, one for self; all bases are shared. Implemented by
	every UlamClass.
    */
    virtual u32 GetBaseClassCount() const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
	Returns the number of direct base classes + self, two minimum:
	one for Self, one for Super.  Implemented by every UlamClass.
    */
    virtual u32 GetDirectBaseClassCount() const
    {
      FAIL(ILLEGAL_STATE);
    }

    /**
	Returns the THE_INSTANCE of ith baseclass, where self is zero,
	followed by direct bases in the order listed, then by
	inherited bases; Implemented by every UlamClass.
    */
    virtual UlamClass<EC> * GetOrderedBaseClassAsUlamClass(u32 ith) const
    {
      FAIL(ILLEGAL_STATE);
    }


    /**
	Returns true if the argument is a "direct" baseclass (explicit
	in class definition), or unspecified superclass (UrSelf) of
	ourself; o.w. false if self, or "shared" base (ancestor of a
	direct base), or unrelated. Implemented by every UlamClass,
	except localfilescopes.
    */
    virtual bool IsDirectBaseClass(const u32 regid) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Downcast an UlamClass* to an UlamQuark* if possible.  Returns
       null if this is not an UlamQuark.
     */
    virtual const UlamQuark<EC>* AsUlamQuark() const
    {
      return 0;
    }

    virtual UlamQuark<EC>* AsUlamQuark()
    {
      return 0;
    }

    /**
       Downcast an UlamClass* to an UlamElement* if possible.  Returns
       null if this is not an UlamElement.
     */
    virtual const UlamElement<EC>* AsUlamElement() const
    {
      return 0;
    }

    virtual UlamElement<EC>* AsUlamElement()
    {
      return 0;
    }

    /**
       Downcast a const UlamClass* to a const UlamTransient* if possible.
       Returns null if this is not an UlamTransient.
     */
    virtual bool IsUlamTransient() const
    {
      return false;
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
    static s32 PositionOfDataMember(const UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName) ;

    /**
       Find the ancestor of quark in this element's family tree by comparing the
       address of its static THE_INSTANCE type, if any exist.

       \return true if they are related;

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual bool internalCMethodImplementingIs(const UlamClass<EC> * cptrarg) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }


    /**
       Compare type of this element to atom

       \return true if they are related;

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual bool internalCMethodImplementingIs(const typename EC::ATOM_CONFIG::ATOM_TYPE& targ) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Compare this class registration number to table of relative reg nums

       \return true if they are related;

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual bool internalCMethodImplementingIs(const u32 regid) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Discover if quark Type, specified by its \c
       INSTANCE address, in an UlamElement specified by its \c type
       number, if such as UlamElement exists and inherits from such a quark.

       \param type an element type number, hopefully of an UlamElement

       \param quarkTypeName the name of the type to search for in
              the ancestors of the found UlamElement.

       \return A return value of true indicates the given \c type is
               related to type of quark.

       \sa T::ATOM_FIRST_STATE_BIT
       \sa internalCMethodImplementingIs
     */
    static bool IsMethod(const UlamContext<EC>& uc, u32 type, const UlamClass<EC> * classPtr);

    /**
       Discover if base class Type, specified by its \c
       INSTANCE address, in an UlamElement specified by its \c type
       number, if such as UlamElement exists and inherits from such a base class,
       return its relative starting position.

       \param type an element type number, hopefully of an UlamElement

       \param quarkTypeName the name of the type to search for in
              the ancestors of the found UlamElement.

       \return A return value of positive start position indicates the given \c type is
               related to type of base class. A negative value means they are not related.

       \sa T::ATOM_FIRST_STATE_BIT
       \sa internalCMethodImplementingIs
     */
    static s32 GetRelativePositionOfBaseClass(const UlamContext<EC>& uc, u32 type, const UlamClass<EC> * baseclassPtr);


    virtual s32 internalCMethodImplementingGetRelativePositionOfBaseClass(const UlamClass<EC> * cptrarg) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    virtual s32 internalCMethodImplementingGetRelativePositionOfBaseClass(const u32 regid) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }


    /**
       Return vtable of this element, or NULL if there isn't one.

       \return ptr to function pointers
     */
    virtual VfuncPtr getVTableEntry(u32 idx) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
      return (VfuncPtr) NULL;
    }

    /**
       Return vtable of this class

       \return UlamClass pointer of override class in this class
     */
    virtual const UlamClass<EC> * getVTableEntryUlamClassPtr(u32 idx) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
      return 0;
    }


    static VfuncPtr GetVTableEntry(const UlamContext<EC>& uc, u32 atype, u32 idx);

    virtual u32 GetVTStartOffsetForClassByRegNum(u32 rn) const
    {
      FAIL(ILLEGAL_STATE); // culam should always have overridden this method
      return 0;
    }

    static void PureVirtualFunctionCalled()
    {
      FAIL(PURE_VIRTUAL_CALLED);
    }

    /**
       Specify the mangled name of this class.  To be
       overridden by subclasses of UlamClass.

       \return a pointer to a statically-allocated const char *.
       Never returns NULL.

     */
    virtual const char * GetMangledClassName() const = 0;

    /**
       String of the mangled name of this class.  To be
       overridden by subclasses of UlamClass.

       \return an index to a statically-allocated ulam String.
     */
    virtual u32 GetMangledClassNameAsStringIndex() const = 0;

    /**
       String of the name of this class.  To be overridden by
       subclasses of UlamClass.  Four variations for template instances
       depending on argument values.

       \return an index to a statically-allocated ulam String.
     */
    virtual u32 GetUlamClassNameAsStringIndex(bool templateParameters, bool templateValues) const = 0;

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

    void PrintClassMembers(const UlamClassRegistry<EC> & ucr,
			   ByteSink & bs,
			   const BitStorage<EC>& stg,
			   u32 flags,
			   u32 baseStatePos,
                           u32 indent = 0) const ;

    void PrintClassMembers(const UlamClassRegistry<EC> & ucr,
			   ByteSink & bs,
			   const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
			   u32 flags,
			   u32 baseStatePos,
                           u32 indent = 0) const ;

    static void addHex(ByteSink & bs, u64 val) ;
    static void addASCII(ByteSink & bs, u64 val) ;

    virtual u32 GetRegistrationNumber() const = 0;

    virtual bool IsTheEmptyClass() const { return false; }

    UlamClass() { }

  private:
  };

} // MFM

#include "UlamClass.tcc"

#endif /* ULAMCLASS_H */
