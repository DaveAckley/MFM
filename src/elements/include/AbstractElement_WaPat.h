/*                                              -*- mode:C++ -*-
  AbstractElement_WaPat.h Common attributes of inhabitants of Wa-Pat
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
  \file AbstractElement_WaPat.h Common attributes of inhabitants of Wa-Pat
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTELEMENT_WAPAT_H
#define ABSTRACTELEMENT_WAPAT_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{
  template <class CC>
  class AbstractElement_WaPat : public Element<CC>
  {
   public:
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      //////
      // Element state fields

      BIRTH_AGE_POS = P3Atom<P>::P3_STATE_BITS_POS,
      BIRTH_AGE_LEN = 8,

      CURRENT_AGE_POS = BIRTH_AGE_POS + BIRTH_AGE_LEN,
      CURRENT_AGE_LEN = 8,

      WAPAT_FIRST_FREE_POS = CURRENT_AGE_POS + CURRENT_AGE_LEN
    };

    typedef BitField<BitVector<BITS>, VD::U32, CURRENT_AGE_LEN, CURRENT_AGE_POS> AFCurrentAge;
    typedef BitField<BitVector<BITS>, VD::U32, BIRTH_AGE_LEN, BIRTH_AGE_POS> AFBirthAge;

    u32 GetCurrentAge(const T& us) const
    {
      return AFCurrentAge::Read(this->GetBits(us));
    }

    void SetCurrentAge(T& us, const u32 age) const
    {
      AFCurrentAge::Write(this->GetBits(us), age);
    }

    u32 GetBirthAge(const T& us) const
    {
      return AFBirthAge::Read(this->GetBits(us));
    }

    void SetBirthAge(T& us, const u32 age) const
    {
      AFBirthAge::Write(this->GetBits(us), age);
    }

    AbstractElement_WaPat(const UUID & uuid) : Element<CC>(uuid)
    { }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

  };
}

#endif /* ABSTRACTELEMENT_WAPAT_H */
