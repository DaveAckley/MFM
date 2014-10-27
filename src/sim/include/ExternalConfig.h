/*                                              -*- mode:C++ -*-
  ExternalConfig.h Support for grid configurations on the drive
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
  \file ExternalConfig.h Support for grid configurations on the drive
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIG_H
#define EXTERNALCONFIG_H

#include "Grid.h"
#include "OverflowableCharBufferByteSink.h"  /* For OString16 */
#include "LineCountingByteSource.h"
#include "ByteSink.h"
#include "ElementRegistry.h"

namespace MFM
{

  template<typename> class ConfigFunctionCall;

  /**
   * Structure for reading and writing the current grid configuration
   */
  template<class GC>
  class ExternalConfig
  {
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    typedef typename CC::ATOM_TYPE T;
    enum { BPA = P::BITS_PER_ATOM };

  public:

    /*
     * Construct a new ExternalConfig referencing a specified Grid
     *
     * @param grid The grid to read from or write to.
     */
    ExternalConfig(Grid<GC>& grid);

    void SetByteSource(ByteSource & byteSource, const char * label) ;

    void SetErrorByteSink(ByteSink & errorsTo) ;

    /**
     * Reads a configuration to the grid specified at construction.
     * \returns true if the loading succeeded; false if the
     * configuration file is invalid (in which case an error has been
     * printed to \a errors, and the grid may be in a partially-loaded
     * or otherwise incomplete state.)
     */
    bool Read();

    /**
     * Writes the current grid configuration to the give \a ByteSink.
     */
    void Write(ByteSink & byteSink);

    void RegisterFunction(ConfigFunctionCall<GC> & fc) ;

    bool RegisterElement(const UUID & uuid, OString16 & nick) ;

    Element<CC> * LookupElement(const OString16 & nick) const ;

    bool PlaceAtom(const Element<CC> & elt, s32 x, s32 y, const char* dataStr) ;

    bool PlaceAtom(const Element<CC> & elt, s32 x, s32 y, const BitVector<BPA> & bv) ;

    void SetTileToExecuteOnly(const SPoint& tileLoc, bool value);

    LineCountingByteSource & GetByteSource()
    {
      return m_in;
    }

  private:
    LineCountingByteSource m_in;
    ByteSink * m_errorsTo;

    /**
     * The Grid to read from or write to.
     */
    Grid<GC>& m_grid;

    /**
     * The ElementRegistry to lookup UUIDs in.
     */
    ElementRegistry<CC>& m_elementRegistry;

#define MAX_REGISTERED_FUNCTIONS 64
    ConfigFunctionCall<GC> * (m_registeredFunctions[MAX_REGISTERED_FUNCTIONS]);
    u32 m_registeredFunctionCount;

#define MAX_REGISTERED_ELEMENTS 64
    struct RegElt {
      UUID m_uuid;
      OString16 m_nick;
      Element<CC> * m_element;
    } m_registeredElements[MAX_REGISTERED_ELEMENTS];
    u32 m_registeredElementCount;

  };
}

#include "ConfigFunctionCall.h"
#include "ExternalConfig.tcc"

#endif /* EXTERNALCONFIG_H */
