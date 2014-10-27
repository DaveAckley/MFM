/*                                              -*- mode:C++ -*-
  Element_Shark.h An element that acts similarly to a Wa-Tor shark
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
  \file Element_Shark.h An element that acts similarly to a Wa-Tor shark
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_SHARK_H
#define ELEMENT_SHARK_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Atom.h"
#include "AbstractElement_WaPat.h"
#include "Element_Fish.h"

namespace MFM
{

  template <class CC>
  class Element_Shark : public AbstractElement_WaPat<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SHARK_ENERGY_POS = AbstractElement_WaPat<CC>::WAPAT_FIRST_FREE_POS,
      SHARK_ENERGY_LEN = 9,

      MAX_SHARK_ENERGY = (1<<SHARK_ENERGY_LEN) - 1,

      INITIAL_DEFAULT_BIRTH_AGE = 15,
      DEFAULT_ENERGY_PER_FISH = 8
    };

    ElementParameterS32<CC> m_sharkBirthAge;
    ElementParameterS32<CC> m_sharkEnergyPerFish;

  public:

    enum
    {
      ELEMENT_VERSION = 1
    };

    typedef BitField<BitVector<BITS>, VD::U32, SHARK_ENERGY_LEN, SHARK_ENERGY_POS> AFSharkEnergy;

    u32 GetSharkEnergy(const T& us) const
    {
      return AFSharkEnergy::Read(this->GetBits(us));
    }

    void SetSharkEnergy(T& us, const u32 age) const
    {
      AFSharkEnergy::Write(this->GetBits(us), age);
    }

    static Element_Shark THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Shark() :
      AbstractElement_WaPat<CC>(MFM_UUID_FOR("Shark", ELEMENT_VERSION)),
      m_sharkBirthAge(this, "age", "Birth Age",
                      "Number of events for a shark to mature",
                      1, INITIAL_DEFAULT_BIRTH_AGE, 100/*, 1*/),
      m_sharkEnergyPerFish(this, "energy", "Energy Per Fish",
                           "Life events gained per fish eaten",
                           1, DEFAULT_ENERGY_PER_FISH, 50/*, 1*/)
    {
      Element<CC>::SetAtomicSymbol("Sh");
      Element<CC>::SetName("Shark");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);

      this->SetBirthAge(defaultAtom, m_sharkBirthAge.GetValue());
      this->SetCurrentAge(defaultAtom, 0);
      this->SetSharkEnergy(defaultAtom, m_sharkEnergyPerFish.GetValue());

      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffff2244;

    }

    // Non-diffusable
    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      T self = window.GetCenterAtom();
      WindowScanner<CC> scanner(window);

      SPoint fishRel;
      u32 fishCount = 0;

      SPoint emptyRel;
      u32 emptyCount = 0;

      u32 energy = GetSharkEnergy(self);
      bool starved = energy == 0;

      if (starved)
      {
        window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        return;
      }

      energy = energy - 1;
      this->SetSharkEnergy(self, energy);

      u32 age = this->GetCurrentAge(self);
      // Don't use genetic birth age (yet): bool reproable = age >= this->GetBirthAge(self);
      bool reproable = age >= (u32) m_sharkBirthAge.GetValue();

      if (!reproable)
      {
        this->SetCurrentAge(self, 1 + age);
      }

      scanner.FindRandomAtoms(1, 2,
                              &fishRel, Element_Fish<CC>::THE_INSTANCE.GetType(), &fishCount,
                              &emptyRel, Element_Empty<CC>::THE_INSTANCE.GetType(), &emptyCount);

      if (fishCount > 0)   // Eating
      {
        energy = MIN((u32) MAX_SHARK_ENERGY, energy + m_sharkEnergyPerFish.GetValue());

        if (reproable)
        {
          energy = energy / 2;  // parent and kid split it
        }

        this->SetSharkEnergy(self, energy);

        if (reproable)
        {
          this->SetCurrentAge(self,0);      // reset age counter
          window.SetCenterAtom(self); // and clone a kid
        }
        else               // or leave empty behind
        {
          window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        }
        window.SetRelativeAtom(fishRel,self); // move or repro
      }
      else if (emptyCount > 0)  // If can't eat, but can move
      {
        if (reproable)     // and leave kid behind
        {
          energy = energy / 2;  // parent and kid split available energy
          this->SetSharkEnergy(self, energy);
          this->SetCurrentAge(self,0);
          window.SetCenterAtom(self);
        }
        else               // or leave empty behind
        {
          window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        }
        window.SetRelativeAtom(emptyRel,self); // move or repro
      }
      else                 // Can't move
      {
        window.SetCenterAtom(self);  // But can age and get hungry
      }
    }
  };

  template <class CC>
  Element_Shark<CC> Element_Shark<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_SHARK_H */
