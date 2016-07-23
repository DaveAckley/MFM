/*                                              -*- mode:C++ -*-
  ExternalConfigSectionDriver.h Support for persisting driver information
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
  \file ExternalConfigSectionDriver.h Support for persisting driver information
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGSECTIONDRIVER_H
#define EXTERNALCONFIGSECTIONDRIVER_H

#include "ExternalConfig.h"

namespace MFM
{
  template <class GC> class ExternalConfigSectionDriver; // FORWARD

  template<class GC>
  class ConfigFunctionCallDriver : public ConfigFunctionCall<GC>
  {
  public:
    ConfigFunctionCallDriver(const char * name, ExternalConfigSectionDriver<GC> & ecsg)
      : ConfigFunctionCall<GC>(name)
      , m_ecsg(ecsg)
    { }
    virtual bool Parse() = 0;

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply() { /* Work already done */ }

  private:
    ExternalConfigSectionDriver<GC> & m_ecsg;

  protected:
    ExternalConfigSectionDriver<GC> & GetECSG() { return m_ecsg; }
  };

  template <class GC>
  struct FunctionCallRuntimeData : public ConfigFunctionCallDriver<GC>
  {
    FunctionCallRuntimeData(ExternalConfigSectionDriver<GC> & ec) ;
    virtual bool Parse() ;
  };

  template<class GC>
  class ExternalConfigSectionDriver : public ExternalConfigSection<GC>
  {

  public:
    virtual const char * GetSectionName() const
    {
      return "Driver";
    }

    /*
     * Construct a new ExternalConfigSectionDriver referencing a specified Driver
     *
     * @param grid The grid to read from or write to.
     */
    ExternalConfigSectionDriver(ExternalConfig<GC>& ec, AbstractDriver<GC>& driver);

    virtual void WriteSection(ByteSink & byteSink);

    virtual bool ContinueOnErrors() const
    {
      return true;
    }

    virtual void Reset() ;

    virtual bool ReadInit() ;

    virtual bool ReadFinalize() ;

    AbstractDriver<GC> & GetDriver()
    {
      return m_driver;
    }

  private:

    /**
     * The Driver to read from or write to.
     */
    AbstractDriver<GC>& m_driver;

    FunctionCallRuntimeData<GC> m_fcRuntimeData;

  };
}

#include "ExternalConfigSectionDriver.tcc"

#endif /* EXTERNALCONFIGSECTIONDRIVER_H */
