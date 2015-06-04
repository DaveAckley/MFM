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
#include "ExternalConfigSection.h"

namespace MFM
{

  template<class GC> class AbstractDriver; // FORWARD

  /**
   * Structure for reading and writing the current grid configuration
   */
  template<class GC>
  class ExternalConfig
  {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { BPA = AC::BITS_PER_ATOM };
    enum { TILE_SIDE = GC::TILE_SIDE };
    enum { EVENT_WINDOW_RADIUS = EC::EVENT_WINDOW_RADIUS };

  public:
    // MFS_VERSION = 1 Original minimal version
    // MFS_VERSION = 2 (2014/2015) Add grid size info
    // MFS_VERSION = 3 (2015) Add sections
    enum { MFS_VERSION = 3 };

    /*
     * Construct a new ExternalConfig associated with a specific driver
     *
     * @param driver The driver to read from or write to.
     */
    ExternalConfig(AbstractDriver<GC>& driver);

    void SetByteSource(ByteSource & byteSource, const char * label) ;

    void SetErrorByteSink(ByteSink & errorsTo) ;

    void RegisterSection(ExternalConfigSection<GC>& ecs);


    /**
     * Reads a configuration to the driver specified at construction.
     * \returns true if the loading succeeded; false if the
     * configuration file is invalid (in which case an error has been
     * printed to \a errors, and the driver may be in a
     * partially-loaded or otherwise incomplete state.)
     */
    bool Read();

    /**
     * Writes the current driver configuration to the given \a ByteSink.
     */
    void Write(ByteSink & byteSink);

    LineCountingByteSource & GetByteSource()
    {
      return m_in;
    }

  private:
    LineCountingByteSource m_in;
    ByteSink * m_errorsTo;

    /**
     * The driver to read from or write to.
     */
    AbstractDriver<GC>& m_driver;

    static const u32 MAX_REGISTERED_SECTIONS = 10;
    ExternalConfigSection<GC> * (m_registeredSections[MAX_REGISTERED_SECTIONS]);
    u32 m_registeredSectionCount;

  };
}

#include "ExternalConfig.tcc"

#endif /* EXTERNALCONFIG_H */
