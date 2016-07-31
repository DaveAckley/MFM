/*                                              -*- mode:C++ -*-
  ExternalConfigSectionGUI.h Support for persisting MFMS GUI information
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
  \file ExternalConfigSectionGUI.h Support for persisting MFMS GUI information
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGSECTIONGUI_H
#define EXTERNALCONFIGSECTIONGUI_H

#include "ExternalConfig.h"
#include "Panel.h"

namespace MFM
{
  template <class GC> class AbstractGUIDriver; // FORWARD
  template <class GC> class ExternalConfigSectionGUI; // FORWARD

  template<class GC>
  class ConfigFunctionCallGUI : public ConfigFunctionCall<GC>
  {
  public:
    ConfigFunctionCallGUI(const char * name, ExternalConfigSectionGUI<GC> & ecsg)
      : ConfigFunctionCall<GC>(name)
      , m_ecsg(ecsg)
    { }
    virtual bool Parse() = 0;

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply() { /* Work already done */ }

  private:
    ExternalConfigSectionGUI<GC> & m_ecsg;

  protected:
    ExternalConfigSectionGUI<GC> & GetECSG() { return m_ecsg; }
  };

  template <class GC>
  struct FunctionCallScreenConfig : public ConfigFunctionCallGUI<GC>
  {
    FunctionCallScreenConfig(ExternalConfigSectionGUI<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallPanelConfig : public ConfigFunctionCallGUI<GC>
  {
    FunctionCallPanelConfig(ExternalConfigSectionGUI<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallPanelProperty : public ConfigFunctionCallGUI<GC>
  {
    FunctionCallPanelProperty(ExternalConfigSectionGUI<GC> & ec) ;
    virtual bool Parse() ;
  };

  template<class GC>
  class ExternalConfigSectionGUI : public ExternalConfigSection<GC>
  {

  public:
    virtual const char * GetSectionName() const
    {
      return "GUI";
    }

    virtual bool ContinueOnErrors() const
    {
      return true;
    }

    virtual void Reset() ;

    virtual bool ReadFinalize() ;

    /*
     * Construct a new ExternalConfigSectionGUI referencing a specified Grid
     *
     * @param grid The grid to read from or write to.
     */
    ExternalConfigSectionGUI(ExternalConfig<GC>& ec, AbstractGUIDriver<GC>& guid);

    virtual void WriteSection(ByteSink & byteSink);

    virtual bool ReadInit() ;

    AbstractGUIDriver<GC> & GetDriver()
    {
      return m_guid;
    }

    void SetLastPanelConfig(Panel * p)
    {
      m_lastPanelConfig = p;
    }

    Panel * GetLastPanelConfig()
    {
      return m_lastPanelConfig;
    }

  private:

    Panel * m_lastPanelConfig;

    /**
     * The AbstractGUIDriver to configure or read from.
     */
    AbstractGUIDriver<GC>& m_guid;

    FunctionCallPanelConfig<GC> m_fcPanelConfig;
    FunctionCallPanelProperty<GC> m_fcPanelProperty;
    FunctionCallScreenConfig<GC> m_fcScreenConfig;

  };
}

#include "ExternalConfigSectionGUI.tcc"

#endif /* EXTERNALCONFIGSECTIONGUI_H */
