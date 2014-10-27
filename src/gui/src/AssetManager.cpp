/* -*- C++ -*- */
#include "AssetManager.h"

namespace MFM
{
  SDL_Surface* AssetManager::surfaces[ASSET_COUNT] = { NULL };

  TTF_Font* AssetManager::fonts[FONT_ASSET_COUNT] = { NULL };

  bool AssetManager::initialized = false;
}
