#include "ITCIcons.h"
#include "Logger.h"

namespace MFM {
  ITCIcons::ITCIcons()
    : mZSheet(0)
  { }

  void ITCIcons::init(SDL_Surface * screen) {
    const char * ZSHEET_PATH = "/home/t2/MFM/res/images/t2viz-icons-ZSHEET.png";
    const char * filename = ZSHEET_PATH;
    SDL_Surface* loaded = NULL;
    SDL_Surface* opped = NULL;

    loaded = IMG_Load(filename);

    if(loaded) {
      opped = SDL_DisplayFormatAlpha(loaded);

      SDL_FreeSurface(loaded);

      LOG.Debug("Surface %s loaded: %p, opped: %p", filename, loaded, opped);
    } else {
      LOG.Error("Image %s not loaded : %s",
                filename,
                IMG_GetError());
    }

    mZSheet = opped;
  }

  bool ITCIcons::getIconPosAndSize(const u32 dirIdx, const u32 funcIdx, const u32 sizeIdx, UPoint & pos, UPoint & size) {
    if (!mZSheet) return false;
    if (dirIdx >= ICON_DIR_IDX_COUNT) return false;
    if (funcIdx >= ICON_FUNC_IDX_COUNT) return false;
    if (sizeIdx > ICON_SIZE_IDX_COUNT) return false;
    // We need the sum of the even integers between 2*sizeIdx and 32..
    // Do it as sum of integers between sizeIdx and 16 then double.
    const u32 a = sizeIdx, b = 16;
    const u32 side = 2*sizeIdx;
    const u32 pixUp = (b-a+1)*(a+b);  // sum of ints from a..b == (b-a+1)*(a+b)/2
    const u32 dirPixOver = (side*dirIdx);
    const u32 pixOver = (side*ICON_DIR_IDX_COUNT)*funcIdx + dirPixOver;

    pos.SetX(pixOver);
    pos.SetY(mZSheet->h - pixUp);

    size.SetX(side);
    size.SetY(side);

    return true;
  }
    
}
