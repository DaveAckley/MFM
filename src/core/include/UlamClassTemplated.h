/*                                              -*- mode:C++ -*-
  UlamClassTemplated.h An abstract base class for ULAM quarks and elements
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
  \file UlamClassTemplated.h An abstract base class for ULAM quarks and elements
  \author David H. Ackley.
  \author Elenas S. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */

#ifndef ULAMCLASSTEMPLATED_H
#define ULAMCLASSTEMPLATED_H

#include "itype.h"
#include "Element.h"
#include "UlamContext.h"
#include "UlamClass.h"

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


namespace MFM {

  //class UlamClass; // FORWARD

  template <class EC>
  class UlamClassTemplated : public UlamClass
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
    static s32 PositionOfDataMember(UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName) ;


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
    static bool IsMethod(UlamContext<EC>& uc, u32 type, const char * quarkTypeName);

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

    static VfuncPtr GetVTableEntry(UlamContext<EC>& uc, const typename EC::ATOM_CONFIG::ATOM_TYPE& atom, u32 atype, u32 idx);

  };

} //MFM

#include "UlamClassTemplated.tcc"

#endif /* ULAMCLASSTEMPLATED_H */
