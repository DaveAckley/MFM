#ifndef TEST_COMMON_H      /* -*- C++ -*- */
#define TEST_COMMON_H

#include "Grid.h"
#include "EventConfig.h"
#include "P3Atom.h"
#include "ElementTable.h"
#include "EventWindow.h"
#include "SizedTile.h"

namespace MFM {

  /* Some types for us to test */
  //  Deprecating P1Atom Sun Jul 27 18:00:05 2014
  // typedef P1Atom<TestParamConfig> TestAtom;
  //  Deprecating P0Atom for MFMv3 Fri Jan 30 13:45:35 2015
  // typedef P0Atom<TestParamConfig> TestAtom;
  typedef P3Atom TestAtom;
  typedef Site<P3AtomConfig> TestSite;
  typedef EventConfig<TestSite, 4> TestEventConfig;

  typedef GridConfig<TestEventConfig,40,1000> TestGridConfig;
  typedef Grid<TestGridConfig> TestGrid;
  typedef TestGrid::GridTile TestTile;

  typedef ElementTable<TestEventConfig> TestElementTable;
  typedef EventWindow<TestEventConfig> TestEventWindow;

} /* namespace MFM */

#endif /*TEST_COMMON_H*/
