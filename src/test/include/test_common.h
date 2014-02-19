#ifndef TEST_COMMON_H      /* -*- C++ -*- */
#define TEST_COMMON_H

#include "grid.h"
#include "p1atom.h"
#include "elementtable.h"
#include "eventwindow.h"
#include "tile.h"

namespace MFM {

/* Some types for us to test */
typedef Grid<P1Atom,4,3,3> GridP1Atom;
typedef ElementTable<P1Atom,4,8> ElementTableP1Atom;
typedef EventWindow<P1Atom,4> EventWindowP1Atom;
typedef Tile<P1Atom,4> TileP1Atom;
} /* namespace MFM */

#endif /*TEST_COMMON_H*/

