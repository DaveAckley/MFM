#ifndef TEST_COMMON_H      /* -*- C++ -*- */
#define TEST_COMMON_H

#include "Grid.h"
#include "P1Atom.h"
#include "ParamConfig.h"
#include "ElementTable.h"
#include "EventWindow.h"
#include "Tile.h"

namespace MFM {

  /* Some types for us to test */
  typedef ParamConfig<> TestParamConfig;
  typedef P1Atom<TestParamConfig> TestAtom;
  typedef CoreConfig<TestAtom, TestParamConfig> TestCoreConfig;

  typedef GridConfig<TestCoreConfig,4,3> TestGridConfig;
  typedef Grid<TestGridConfig> TestGrid;
  typedef ElementTable<TestCoreConfig> TestElementTable;
  typedef EventWindow<TestCoreConfig> TestEventWindow;
  typedef Tile<TestCoreConfig> TestTile;
} /* namespace MFM */

#endif /*TEST_COMMON_H*/

