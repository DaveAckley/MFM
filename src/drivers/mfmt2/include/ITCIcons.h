/* -*- C++ -*- */
#ifndef ITCICONS_H
#define ITCICONS_H

#include "SDL_image.h"
#include "SDL.h"
#include "SDL_ttf.h"

#include "itype.h"
#include "Point.h"

namespace MFM { 
  struct ITCIcons {
    enum {
      ICON_DIR_WEST_IDX = 0,
      ICON_DIR_NORTH_IDX,
      ICON_DIR_EAST_IDX,
      ICON_DIR_SOUTH_IDX,
      ICON_DIR_IDX_COUNT,

      ICON_FUNC_RECV_IDX = 0,
      ICON_FUNC_SEND_IDX,
      ICON_FUNC_CLOSED_IDX,
      ICON_FUNC_ERROR_IDX,
      ICON_FUNC_EMPTY_IDX,
      ICON_FUNC_IDX_COUNT,

      ICON_SIZE_IDX_COUNT = 16
    };

    bool getIconPosAndSize(const u32 dirIdx, const u32 funcIdx, const u32 sizeIdx, UPoint & pos, UPoint & size) ;

    ITCIcons() ;
    // We don't use screen it's just a reminder to set up SDL before coming here..
    void init(SDL_Surface * screen) ;
    SDL_Surface* mZSheet;

    SDL_Surface * getZSheet() { return mZSheet; }
  };
}

#endif /* ITCICONS_H */
