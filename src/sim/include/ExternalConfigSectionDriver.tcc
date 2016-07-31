/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AtomSerializer.h"
#include <string.h>
#include <ctype.h>

namespace MFM
{
  template <class GC>
  FunctionCallRuntimeData<GC>::FunctionCallRuntimeData(ExternalConfigSectionDriver<GC> & ec)
    : ConfigFunctionCallDriver<GC>("RuntimeData", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallRuntimeData<GC>::Parse()
  {
    ExternalConfigSectionDriver<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    AbstractDriver<GC> & ad = ec.GetDriver();
    if (!ad.LoadRuntimeData(in))
      return false;
    return 1 == in.Scanf(")");
  }

  template<class GC>
  ExternalConfigSectionDriver<GC>::ExternalConfigSectionDriver(ExternalConfig<GC>& ec, AbstractDriver<GC>& driver)
    : ExternalConfigSection<GC>(ec)
    , m_driver(driver)
    , m_fcRuntimeData(*this)
  { }

  template<class GC>
  void ExternalConfigSectionDriver<GC>::Reset()
  {
    /* no section-level state */
  }

  template<class GC>
  bool ExternalConfigSectionDriver<GC>::ReadInit()
  {
    return true;
  }

  template<class GC>
  bool ExternalConfigSectionDriver<GC>::ReadFinalize()
  {
    return true;
  }

  template<class GC>
  void ExternalConfigSectionDriver<GC>::WriteSection(ByteSink& byteSink)
  {
    AbstractDriver<GC>& ad = this->GetDriver();

    /* Stash runtime data affecting stats and the adaptive scheduler. */
    byteSink.Printf("RuntimeData(");
    ad.SaveRuntimeData(byteSink);
    byteSink.Printf(")\n");
  }

}
