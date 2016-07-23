/*                                              -*- mode:C++ -*-
  ExternalConfigSection.h Support for sectioned configuration information
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file ExternalConfigSection.h Support for sectioned configuration information
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGSECTION_H
#define EXTERNALCONFIGSECTION_H

#include "OverflowableCharBufferByteSink.h"  /* For OString16 */
#include "LineCountingByteSource.h"
#include "ByteSink.h"

namespace MFM
{

  template<class GC> class ExternalConfig; // FORWARD
  template<typename> class ConfigFunctionCall; // FORWARD

  /**
   * Structure for reading and writing a related set of simulation
   * configuration information
   */
  template<class GC>
  class ExternalConfigSection
  {
  public:
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    ExternalConfigSection(ExternalConfig<GC> & ec) ;
    virtual ~ExternalConfigSection() { }

    virtual const char * GetSectionName() const = 0;

    virtual bool ContinueOnErrors() const = 0;

    /**
       Clear out section-specific info.  Called once per .mfs load.
     */
    virtual void Reset() = 0;

    /**
       Method called before section reading begins.  Return false to
       abort the read.
     */
    virtual bool ReadInit()
    {
      // Successful by default
      return true;
    }

    /**
       Method called after section reading completes.  Return false to
       fail the read.
     */
    virtual bool ReadFinalize()
    {
      // Successful by default
      return true;
    }

    /**
     * Reads a configuration section from GetByteSource(), after the
     * opening [sectionname] has already been read, up through and
     * including the closing [/sectionname].  \returns true if the
     * loading succeeded; false if the configuration file is invalid
     * (in which case an error has been printed to GetByteSource(),
     * and any underlying state may be incomplete or otherwise
     * corrupt.)
     */
    bool ReadSection();

    bool IsEnabled() const
    {
      return m_sectionEnabled;
    }

    void SetEnabled(bool value)
    {
      m_sectionEnabled = value;
    }

    /**
     * Writes the current section information to the given
     * \a ByteSink.
     */
    void Write(ByteSink & byteSink) ;

    /**
     * Writes the guts of the current section information to the given
     * \a ByteSink.  The section header has already been written; the
     * section trailer will be written afterwards.
     */
    virtual void WriteSection(ByteSink & byteSink) = 0;

    void RegisterFunction(ConfigFunctionCall<GC> & fc) ;

    LineCountingByteSource & GetByteSource() ;
  private:
    ExternalConfig<GC>& m_ec;

    static const u32 MAX_REGISTERED_FUNCTIONS = 16;
    ConfigFunctionCall<GC> * (m_registeredFunctions[MAX_REGISTERED_FUNCTIONS]);
    u32 m_registeredFunctionCount;

    bool m_sectionEnabled;
  };
}

#include "ExternalConfigSection.tcc"

#endif /* EXTERNALCONFIGSECTION_H */
