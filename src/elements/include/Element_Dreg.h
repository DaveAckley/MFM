/*                                              -*- mode:C++ -*-
  Element_Dreg.h Basic dynamic regulating element
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
  \file Element_Dreg.h Basic dynamic regulating element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_DREG_H
#define ELEMENT_DREG_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"
#include "Element_Res.h"  /* For Element_Res::TYPE */
#include "Element_Wall.h" /* For Element_Wall::TYPE */

namespace MFM
{

#define DREG_VERSION 1

  template <class CC>
  class Element_Dreg : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:

    static s32 m_resOdds;

    static s32 m_dregCreateOdds;


    static s32 m_dregDeleteOdds;


    static s32 m_dregDeleteDregOdds;

  public:

    static Element_Dreg THE_INSTANCE;

    Element_Dreg() : Element<CC>(MFM_UUID_FOR("Dreg", DREG_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Dr");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff505050;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff282828;
    }

    virtual const char* GetDescription() const
    {
      return "Short for \"Dynamic Regulator\", This Atom controls the density of "
             "nearby Atoms by creating RES atoms and deleting nearby atoms.";
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      SPoint dir;
      MDist<R>::get().FillRandomSingleDir(dir, random);

      if (window.IsLiveSite(dir))
      {
        T atom = window.GetRelativeAtom(dir);
        u32 oldType = atom.GetType();

        if(Element_Empty<CC>::THE_INSTANCE.IsType(oldType))
        {
          if(random.OneIn(m_dregCreateOdds))
          {
            atom = Element_Dreg<CC>::THE_INSTANCE.GetDefaultAtom();
          }
          else if(random.OneIn(m_resOdds))
          {
            atom = Element_Res<CC>::THE_INSTANCE.GetDefaultAtom();
          }
        }
        else if(oldType == Element_Dreg::THE_INSTANCE.GetType())
        {
          if(random.OneIn(m_dregDeleteDregOdds))
          {
            atom = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
          }
        }
        else if(oldType != Element_Wall<CC>::TYPE() && random.OneIn(m_dregDeleteOdds))
        {
          atom = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
        }

        if(atom.GetType() != oldType)
        {
          window.SetRelativeAtom(dir, atom);
        }
      }
      this->Diffuse(window);
    }

    s32* GetResOddsPtr()
    {
      return &m_resOdds;
    }

    s32* GetDregCreateOddsPtr()
    {
      return &m_dregCreateOdds;
    }


    s32* GetDregDeleteOddsPtr()
    {
      return &m_dregDeleteOdds;
    }


    s32* GetDregDeleteDregOddsPtr()
    {
      return &m_dregDeleteDregOdds;
    }
  };

  template <class CC>
  Element_Dreg<CC> Element_Dreg<CC>::THE_INSTANCE;

  template <class CC>
  s32 Element_Dreg<CC>::m_resOdds = 200;

  template <class CC>
  s32 Element_Dreg<CC>::m_dregDeleteOdds = 100;

  template <class CC>
  s32 Element_Dreg<CC>::m_dregCreateOdds = 500;

  template <class CC>
  s32 Element_Dreg<CC>::m_dregDeleteDregOdds = 50;
}

#endif /* ELEMENT_DREG_H */
