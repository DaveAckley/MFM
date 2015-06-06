/*                                              -*- mode:C++ -*-
  ExternalConfigSectionMFMS.h Support for persisting MFMS GUI information
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
  \file ExternalConfigSectionMFMS.h Support for persisting MFMS GUI information
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGSECTIONMFMS_H
#define EXTERNALCONFIGSECTIONMFMS_H

#include "ExternalConfig.h"

namespace MFM
{
  template <class GC> class AbstractGUIDriver; // FORWARD
  template <class GC> class ExternalConfigSectionMFMS; // FORWARD

  template<class GC>
  class ConfigFunctionCallMFMS : public ConfigFunctionCall<GC>
  {
  public:
    ConfigFunctionCallMFMS(const char * name, ExternalConfigSectionMFMS<GC> & ecsm)
      : ConfigFunctionCall<GC>(name)
      , m_ecsm(ecsm)
    { }
    virtual bool Parse() = 0;

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply() { /* Work already done */ }

  private:
    ExternalConfigSectionMFMS<GC> & m_ecsm;

  protected:
    ExternalConfigSectionMFMS<GC> & GetECSM() { return m_ecsm; }
  };

  template <class GC>
  struct FunctionCallPanelConfig : public ConfigFunctionCallMFMS<GC>
  {
    FunctionCallPanelConfig(ExternalConfigSectionMFMS<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallPanelProperty : public ConfigFunctionCallMFMS<GC>
  {
    FunctionCallPanelProperty(ExternalConfigSectionMFMS<GC> & ec) ;
    virtual bool Parse() ;
  };

  template<class GC>
  class ExternalConfigSectionMFMS : public ExternalConfigSection<GC>
  {

  public:
    virtual const char * GetSectionName() const
    {
      return "MFMS";
    }

    /*
     * Construct a new ExternalConfigSectionMFMS referencing a specified Grid
     *
     * @param grid The grid to read from or write to.
     */
    ExternalConfigSectionMFMS(ExternalConfig<GC>& ec, AbstractGUIDriver<GC>& guid);

    virtual void WriteSection(ByteSink & byteSink);

    virtual bool ReadInit() ;

    AbstractGUIDriver<GC> & GetDriver()
    {
      return m_guid;
    }

  private:

    /**
     * The AbstractGUIDriver to configure or read from.
     */
    AbstractGUIDriver<GC>& m_guid;

    FunctionCallPanelConfig<GC> m_fcPanelConfig;
    FunctionCallPanelProperty<GC> m_fcPanelProperty;

  };
}

#include "ExternalConfigSectionMFMS.tcc"

#endif /* EXTERNALCONFIGSECTIONMFMS_H */
