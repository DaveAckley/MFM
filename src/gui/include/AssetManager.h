/*                                              -*- mode:C++ -*-
  AssetManager.h SDL_Surface management system
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file AssetManager.h SDL_Surface management system
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "SDL_image.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "Logger.h"
#include "Utils.h"

namespace MFM
{
  enum Asset
  {
    ASSET_SELECTOR_ICON = 0,
    ASSET_ATOM_SELECTOR_ICON,
    ASSET_PENCIL_ICON,
    ASSET_ERASER_ICON,
    ASSET_BRUSH_ICON,
    ASSET_BUCKET_ICON,
    ASSET_XRAY_ICON,
    ASSET_CLONE_ICON,
    ASSET_AIRBRUSH_ICON,
    ASSET_CHECKBOX_ICON_ON,
    ASSET_CHECKBOX_ICON_OFF,
    ASSET_SLIDER_HANDLE,
    ASSET_SELECTOR_ICON_BIG,
    ASSET_ATOM_SELECTOR_ICON_BIG,
    ASSET_PENCIL_ICON_BIG,
    ASSET_ERASER_ICON_BIG,
    ASSET_BRUSH_ICON_BIG,
    ASSET_BUCKET_ICON_BIG,
    ASSET_XRAY_ICON_BIG,
    ASSET_CLONE_ICON_BIG,
    ASSET_AIRBRUSH_ICON_BIG,
    ASSET_COUNT
  };

  enum FontAsset
  {
    FONT_ASSET_ELEMENT = 0,
    FONT_ASSET_ELEMENT_BIG,
    FONT_ASSET_HELPPANEL_BIG,
    FONT_ASSET_HELPPANEL_SMALL,
    FONT_ASSET_LOGGER,
    FONT_ASSET_COUNT
  };

  class AssetManager
  {
  private:

    static SDL_Surface* surfaces[ASSET_COUNT];

    static TTF_Font* fonts[FONT_ASSET_COUNT];

    static bool initialized;

    static SDL_Surface* LoadImage(const char* relativeFilename)
    {
      const u32 BUFFER_SIZE = 1024;
      char filename[BUFFER_SIZE];
      SDL_Surface* loaded = NULL;
      SDL_Surface* opped = NULL;

      if(Utils::GetReadableResourceFile(relativeFilename, filename, BUFFER_SIZE))
      {
        loaded = IMG_Load(filename);

        if(loaded)
        {
          opped = SDL_DisplayFormatAlpha(loaded);

          SDL_FreeSurface(loaded);

          LOG.Debug("Surface %s loaded: %p, opped: %p", filename, loaded, opped);
        }
        else
        {
          LOG.Error("Image %s not loaded : %s",
                    filename,
                    IMG_GetError());
        }
      }
      else
      {
        LOG.Error("Cannot compute relative path to: %s", relativeFilename);
      }

      return opped;
    }

    static TTF_Font* LoadFont(const char* relativePath, u32 size)
    {
      const u32 BUFFER_SIZE = 1024;
      TTF_Font* font = NULL;
      char path[BUFFER_SIZE];

      if(Utils::GetReadableResourceFile(relativePath, path, BUFFER_SIZE))
      {
        font = TTF_OpenFont(path, size);

        if(!font)
        {
          LOG.Error("Font %s not loaded: %s",
                    path,
                    TTF_GetError());
        }

        LOG.Debug("Font %s loaded: %p", relativePath, font);
      }
      else
      {
        LOG.Error("Cannot compute relative path to: %s", relativePath);
      }

      return font;
    }

  public:

    /**
     * Initializes all held Assets. This should only be called once a
     * screen has been created by SDL_SetVideoMode .
     */
    static void Initialize()
    {
      if(!initialized)
      {
        surfaces[ASSET_SELECTOR_ICON] = LoadImage("images/selector_icon.png");
        surfaces[ASSET_ATOM_SELECTOR_ICON] = LoadImage("images/atom_selector_icon.png");
        surfaces[ASSET_PENCIL_ICON] = LoadImage("images/pencil_icon.png");
        surfaces[ASSET_ERASER_ICON] = LoadImage("images/eraser_icon.png");
        surfaces[ASSET_BRUSH_ICON] = LoadImage("images/brush_icon.png");
        surfaces[ASSET_BUCKET_ICON] = LoadImage("images/bucket_icon.png");
        surfaces[ASSET_XRAY_ICON] = LoadImage("images/xray_icon.png");
        surfaces[ASSET_CLONE_ICON] = LoadImage("images/clone_icon.png");
        surfaces[ASSET_AIRBRUSH_ICON] = LoadImage("images/airbrush_icon.png");
        surfaces[ASSET_CHECKBOX_ICON_ON] = LoadImage("images/checkbox_on.png");
        surfaces[ASSET_CHECKBOX_ICON_OFF] = LoadImage("images/checkbox_off.png");
        surfaces[ASSET_SLIDER_HANDLE] = LoadImage("images/slider_handle.png");
        surfaces[ASSET_SELECTOR_ICON_BIG] = LoadImage("images/selector_icon_big.png");
        surfaces[ASSET_ATOM_SELECTOR_ICON_BIG] = LoadImage("images/atom_selector_icon_big.png");
        surfaces[ASSET_PENCIL_ICON_BIG] = LoadImage("images/pencil_icon_big.png");
        surfaces[ASSET_ERASER_ICON_BIG] = LoadImage("images/eraser_icon_big.png");
        surfaces[ASSET_BRUSH_ICON_BIG] = LoadImage("images/brush_icon_big.png");
        surfaces[ASSET_BUCKET_ICON_BIG] = LoadImage("images/bucket_icon_big.png");
        surfaces[ASSET_XRAY_ICON_BIG] = LoadImage("images/xray_icon_big.png");
        surfaces[ASSET_CLONE_ICON_BIG] = LoadImage("images/clone_icon_big.png");
        surfaces[ASSET_AIRBRUSH_ICON_BIG] = LoadImage("images/airbrush_icon_big.png");

        fonts[FONT_ASSET_ELEMENT] = LoadFont("fonts/tiny.ttf", 24);
        fonts[FONT_ASSET_ELEMENT_BIG] = LoadFont("fonts/tiny.ttf", 40);
        fonts[FONT_ASSET_HELPPANEL_BIG] = LoadFont("fonts/tiny.ttf", 26);
        fonts[FONT_ASSET_HELPPANEL_SMALL] = LoadFont("fonts/tiny.ttf", 16);
        fonts[FONT_ASSET_LOGGER] = LoadFont("fonts/tiny.ttf", 15);

        initialized = true;
      }
    }

    /**
     * Frees all held assets, which are subsequently set to NULL.
     */
    static void Destroy()
    {
      if(initialized)
      {
        for(Asset i = ASSET_SELECTOR_ICON; i < ASSET_COUNT; i = (Asset)(i + 1))
        {
          SDL_FreeSurface(surfaces[i]);
          surfaces[i] = NULL;
        }

        for(FontAsset i = FONT_ASSET_ELEMENT; i < FONT_ASSET_COUNT; i = (FontAsset)(i + 1))
        {
          TTF_CloseFont(fonts[i]);
          fonts[i] = NULL;
        }

        initialized = false;
      }
    }

    static SDL_Surface* Get(Asset a)
    {
      return surfaces[a];
    }

    static TTF_Font* Get(FontAsset a)
    {
      return fonts[a];
    }
  };
}

#endif /* ASSETMANAGER_H */
