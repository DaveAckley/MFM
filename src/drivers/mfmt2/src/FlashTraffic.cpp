#include "FlashTraffic.h"
#include "T2FlashTrafficManager.h"

namespace MFM
{
  const BPoint FlashTraffic::dir8Offsets[Dirs::DIR_COUNT] =
    {
     BPoint( 0,+1), // (NT)
     BPoint(+1,+1), // NE
     BPoint(+2, 0), // ET
     BPoint(+1,-1), // SE
     BPoint( 0,-1), // (ST)
     BPoint(-1,-1), // SW
     BPoint(-2, 0), // WT
     BPoint(-1,+1)  // NW
    };

}

