/* -*- C++ -*- */
#include "Test_Common.h"
#include "assert.h"
#include "ExternalConfig_Test.h"

namespace MFM
{

  void ExternalConfig_Test::Test_RunTests()
  {
    Grid<TestGridConfig> grid;
    ExternalConfig<TestGridConfig> cfg(grid, "/home/sixstring982/Documents/Git/MFMv2/configurations/ExternalConfig_Test.input");

    cfg.Read();
  }

}
