/* -*- C++ -*- */
#ifndef SITES_H
#define SITES_H

#include "itype.h"
#include "Dirs.h"
#include "Site.h"
#include "T2Constants.h"
#include "T2Types.h"

namespace MFM {
  struct Sites {

    OurT2Site * getSiteArray() { return &mTheSites[0][0]; }

    OurT2Site& get(UPoint at) {
      MFM_API_ASSERT_ARG(at.BoundedAbove(UPoint(T2TILE_WIDTH-1,T2TILE_HEIGHT-1)));
      return mTheSites[at.GetY()][at.GetX()];
    }

    const OurT2Site& get(UPoint at) const {
      MFM_API_ASSERT_ARG(at.BoundedAbove(UPoint(T2TILE_WIDTH-1,T2TILE_HEIGHT-1)));
      return mTheSites[at.GetY()][at.GetX()];
    }

    Sites() ;

  private:
    typedef OurT2Site OurT2Sites[T2TILE_HEIGHT][T2TILE_WIDTH];
    OurT2Sites mTheSites;
  };
}
#endif /* SITES_H */
