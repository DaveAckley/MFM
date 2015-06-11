/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AbstractGUIDriver.h"
#include "AtomSerializer.h"
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
    ExternalConfigSectionGUI<GC> & ec = this->GetECSM();
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
    ExternalConfigSectionGUI<GC> & ec = this->GetECSM();
    AbstractGUIDriver<GC> & ad = ec.GetDriver();
    LineCountingByteSource & in = ec.GetByteSource();

    in.SkipWhitespace();
    Panel & root = ad.GetRootPanel();
    Panel * target = root.DereferenceFullName(in);
    if (!target)
      return false;
    if (!target->Load(in))
      return false;

    in.SkipWhitespace();
    if (in.Read() != ')')
      return false;

    return true;
  }

  template <class GC>
  FunctionCallPanelProperty<GC>::FunctionCallPanelProperty(ExternalConfigSectionGUI<GC> & ec)
    : ConfigFunctionCallGUI<GC>("PanelProperty", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallPanelProperty<GC>::Parse()
  {

    ExternalConfigSectionGUI<GC> & ec = this->GetECSM();
    //    AbstractGUIDriver<GC> & ad = ec.GetDriver();
    LineCountingByteSource & in = ec.GetByteSource();
    in.SkipWhitespace();

    FAIL(INCOMPLETE_CODE);
  }

  template<class GC>
  ExternalConfigSectionGUI<GC>::ExternalConfigSectionGUI(ExternalConfig<GC>& ec, AbstractGUIDriver<GC>& guid)
    : ExternalConfigSection<GC>(ec)
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
