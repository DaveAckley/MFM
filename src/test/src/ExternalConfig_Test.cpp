/* -*- C++ -*- */
#include "Test_Common.h"
#include "assert.h"
#include "ExternalConfigFunctions.h"
#include "ExternalConfig_Test.h"
#include "ZStringByteSource.h"
#include "FileByteSink.h"  /* For STDERR */
#include "Element_Dreg.h"

namespace MFM
{

  static void TestBasic()
  {
    ElementRegistry<TestCoreConfig> ereg;
    ereg.RegisterElement(Element_Dreg<TestCoreConfig>::THE_INSTANCE);

    Grid<TestGridConfig> grid(ereg);
    ExternalConfig<TestGridConfig> cfg(grid);
    RegisterExternalConfigFunctions<TestGridConfig>(cfg);

    ZStringByteSource zbs("RegisterElement(Dreg-118148428408201406116113117, d)\n"
                          "GA(d,30,31)\n"
                          "GA(d,31,32)\n"
                          "SetParameter(DregResRatio,10)\n");
    cfg.SetByteSource(zbs, "./configurations/ExternalConfig_Test.input");
    cfg.Read();

  }

  void ExternalConfig_Test::Test_RunTests()
  {
    TestBasic();
  }

}
