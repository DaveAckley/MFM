/* -*- mode:C++ -*- */

/*
  AtomSerializer.h A wrapper for (de)serializing an Atom
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
   \file AtomSerializer.h A wrapper for (de)serializing an Atom
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef ATOMSERIALIZER_H
#define ATOMSERIALIZER_H

#include "itype.h"
#include "Atom.h"
#include "BitVector.h"
#include "ByteSerializable.h"

namespace MFM
{

  /**
   * A wrapper class to use for making an Atom ByteSerializable.  We
   * don' make Atom itself ByteSerializable because ByteSerializable
   * has virtual functions and we're not going to pay for a vtable
   * pointer in each Atom..
   */
  template <class CC>
  class AtomSerializer : public ByteSerializable
  {
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };
  private:
    Atom<CC> & m_atom;

  public:
    AtomSerializer(Atom<CC> & atom) : m_atom(atom)
    { }

    Result PrintTo(ByteSink & bs, s32 argument = 0)
    {
      m_atom.m_bits.Print(bs);
      return SUCCESS;
    }

    Result ReadFrom(ByteSource & bs, s32 argument = 0)
    {
      if (m_atom.m_bits.Read(bs))
        return SUCCESS;
      return FAILURE;
    }

    const BitVector<BPA> & GetBits() const
    {
      return m_atom.m_bits;
    }

  };
} /* namespace MFM */

#endif /*ATOMSERIALIZER_H*/
