/* -*- C++ -*- */
#include "Test_Common.h"
#include "assert.h"
#include "ExternalConfigFunctions.h"
#include "ExternalConfig_Test.h"
#include "ZStringByteSource.h"
#include "FileByteSink.h"  /* For STDERR */

namespace MFM
{

  void ExternalConfig_Test::Test_RunTests()
  {
    Grid<TestGridConfig> grid;
    ExternalConfig<TestGridConfig> cfg(grid);
    RegisterExternalConfigFunctions<TestGridConfig>(cfg);

    ZStringByteSource zbs("RegisterElement(4Dreg118201406116113117, d)\n"
                          "GA(d,30,31)\n"
                          "GA(d,31,32)\n"
                          "SetParameter(DregResRatio,10)\n");
    cfg.SetByteSource(zbs, "./configurations/ExternalConfig_Test.input");
    cfg.SetErrorByteSink(STDERR);
    cfg.Read();
  }

}
