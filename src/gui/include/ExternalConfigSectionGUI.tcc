/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AbstractGUIDriver.h"
#include "AtomSerializer.h"
#include "Fail.h"
#include <string.h>
#include <ctype.h>

namespace MFM
{
  template <class GC>
  FunctionCallScreenConfig<GC>::FunctionCallScreenConfig(ExternalConfigSectionGUI<GC> & ec)
    : ConfigFunctionCallGUI<GC>("ScreenConfig", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallScreenConfig<GC>::Parse()
  {
    ExternalConfigSectionGUI<GC> & ec = this->GetECSG();
    AbstractGUIDriver<GC> & ad = ec.GetDriver();
    LineCountingByteSource & in = ec.GetByteSource();

    if (!ad.LoadScreenConfig(in))
      return false;

    in.SkipWhitespace();
    if (in.Read() != ')')
      return false;

    return true;
  }


  template <class GC>
  FunctionCallPanelConfig<GC>::FunctionCallPanelConfig(ExternalConfigSectionGUI<GC> & ec)
    : ConfigFunctionCallGUI<GC>("PanelConfig", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallPanelConfig<GC>::Parse()
  {
    ExternalConfigSectionGUI<GC> & ec = this->GetECSG();
    AbstractGUIDriver<GC> & ad = ec.GetDriver();
    LineCountingByteSource & in = ec.GetByteSource();

    // Clear any prior config
    ec.SetLastPanelConfig(0);

    in.SkipWhitespace();
    Panel & root = ad.GetRootPanel();
    Panel * target = root.DereferenceFullName(in);
    if (!target)
      return false;
#if 0
    if (!target->Load(in))
      return false;
#endif
    in.SkipWhitespace();
    if (in.Read() != ')')
      return false;

    // OK, this one was usable
    ec.SetLastPanelConfig(target);
    return true;
  }

  template <class GC>
  FunctionCallPanelProperty<GC>::FunctionCallPanelProperty(ExternalConfigSectionGUI<GC> & ec)
    : ConfigFunctionCallGUI<GC>("PP", ec)
  {
    ec.RegisterFunction(*this);
  }

  // PP(alphanumerickey="value with newlines and double-quotes %escaped")
  template <class GC>
  bool FunctionCallPanelProperty<GC>::Parse()
  {

    ExternalConfigSectionGUI<GC> & ec = this->GetECSG();
    //    AbstractGUIDriver<GC> & ad = ec.GetDriver();
    LineCountingByteSource & in = ec.GetByteSource();

    Panel * p = ec.GetLastPanelConfig();
    if (!p)
      return in.Msg(Logger::ERROR, "No (successful) PanelConfig before PP");

    OString16 key;
    if (!in.ScanIdentifier(key))
      return in.Msg(Logger::ERROR, "Expected identifier as first argument");

    in.SkipWhitespace();
    if (1 != in.Scanf("="))
      return in.Msg(Logger::ERROR, "Expected '=' after identifier");

    in.SkipWhitespace();
    if (!p->LoadDetails(key.GetZString(), in))
      return in.Msg(Logger::ERROR, "Key '%s' unrecognized or bad value", key.GetZString());

    in.SkipWhitespace();
    if (1 != in.Scanf(")"))
      return in.Msg(Logger::ERROR, "Missing ')'");

    in.SkipWhitespace();

    return true;
  }

  template<class GC>
  ExternalConfigSectionGUI<GC>::ExternalConfigSectionGUI(ExternalConfig<GC>& ec, AbstractGUIDriver<GC>& guid)
    : ExternalConfigSection<GC>(ec)
    , m_lastPanelConfig(0)
    , m_guid(guid)
    , m_fcPanelConfig(*this)
    , m_fcPanelProperty(*this)
    , m_fcScreenConfig(*this)
  { }


  template<class GC>
  bool ExternalConfigSectionGUI<GC>::ReadInit()
  {
    // Nothing needed?
    return true;
  }

  template<class GC>
  void ExternalConfigSectionGUI<GC>::WriteSection(ByteSink& byteSink)
  {
    const Panel & root = GetDriver().GetRootPanel();
    root.SaveAll(byteSink);
    byteSink.Printf("ScreenConfig(");
    GetDriver().SaveScreenConfig(byteSink);
    byteSink.Printf(")\n");
  }

  template<class GC>
  bool ExternalConfigSectionGUI<GC>::ReadFinalize()
  {
    AbstractGUIDriver<GC> & ad = GetDriver();
    ad.ResetScreenSize();
    return true;
  }

}
