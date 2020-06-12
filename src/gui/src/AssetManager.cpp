/* -*- C++ -*- */
#include "AssetManager.h"

namespace MFM
{
  SDL_Surface* AssetManager::surfaces[IMAGE_ASSET_COUNT] = { NULL };

  TTF_Font* AssetManager::fonts[FONT_ASSET_COUNT] = { NULL };

  TTF_Font* AssetManager::zfonts[ZFONT_HEIGHT_COUNT][2] = { NULL };

  bool AssetManager::initialized = false;
}
