/*                                              -*- mode:C++ -*-
  Base.h Fixed storage site components
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
  \file Base.h Fixed storage site components
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef BASE_H
#define BASE_H

#include "Sense.h"
#include "ByteSink.h"
#include "LineCountingByteSource.h"
#include "AtomSerializer.h"

namespace MFM {

  /**
     A Base holds the non-mobile state of a Site.  It is
     a template depending only on an AtomConfig (AC).
  */
  template <class AC>
  class Base
  {
    /**
       Present the AtomConfig in use
     */
    typedef AC ATOM_CONFIG;

    // Extract short names for parameter types
    typedef typename ATOM_CONFIG::ATOM_TYPE T;

    T m_base;
    SiteSensors m_sensory;
    u32 m_paint;

  public:
    Base()
      : m_paint(0xff000000)
    { }

    u32 GetPaint() const
    {
      return m_paint;
    }

    void SetPaint(u32 paint)
    {
      m_paint = paint;
    }

    void SaveConfig(ByteSink& bs, AtomTypeFormatter<AC> & atf) const
    {
      bs.Printf(",");
      atf.PrintAtomType(m_base, bs);
      {
        T tmp = m_base;
        AtomSerializer<AC> as(tmp);
        bs.Printf(",%@", &as);
      }
      bs.Printf(",#%08x",m_paint);

      m_sensory.SaveConfig(bs, atf);
    }

    bool LoadConfig(LineCountingByteSource & bs, AtomTypeFormatter<AC> & atf)
    {
      if (1 != bs.Scanf(",")) return false;

      T defaultAtom;
      if (!atf.ParseAtomType(bs, defaultAtom)) // All we care about is the type..
        return false;

      T tmp_m_base;
      AtomSerializer<AC> as2(tmp_m_base);
      if (2 != bs.Scanf(",%@", &as2))
        return false;

      for (u32 i = T::ATOM_FIRST_STATE_BIT; i < T::BPA; ++i) // merge and pray.  jus'tryin't'elp
      {
        defaultAtom.GetBits().StoreBit(i, tmp_m_base.GetBits().ReadBit(i));
      }

      u32 tmp_m_paint;
      if (3 != bs.Scanf(",#%08x", &tmp_m_paint))
        return false;

      if (!m_sensory.LoadConfig(bs, atf))
        return false;

      m_base = defaultAtom;
      m_paint = tmp_m_paint;

      return true;
    }

    void PutBaseAtom(const T & newBase) { m_base = newBase; }
    T & GetBaseAtom() { return m_base; }
    const T & GetBaseAtom() const { return m_base; }

    SiteSensors & GetSensory() { return m_sensory; }
    const SiteSensors & GetSensory() const { return m_sensory; }
  };

} /* namespace MFM */

#endif /*BASE_H*/
