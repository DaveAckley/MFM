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
#include "UlamContext.h"
#include "UlamClassRegistry.h"

namespace MFM
{

  struct UlamClassDataMemberInfo; //forward

  struct UlamTypeInfo; //forward

  struct UlamClass
  {
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

    template <class EC>
    void PrintClassMembers(const UlamClassRegistry & ucr,
			   ByteSink & bs,
			   const typename EC::ATOM_CONFIG::ATOM_TYPE& atom,
			   u32 flags,
			   u32 baseStatePos) const ;

    static void addHex(ByteSink & bs, u64 val) ;
  };

} // MFM

#include "UlamClass.tcc"

#endif /* ULAMCLASS_H */
