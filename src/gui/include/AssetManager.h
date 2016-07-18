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
#include "Point.h"
#include "Rect.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM
{
  enum ImageAsset
  {
    IMAGE_ASSET_MASTER_ICON_ZSHEET,

    // MUST REMAIN LAST
    IMAGE_ASSET_NONE,
    IMAGE_ASSET_COUNT = IMAGE_ASSET_NONE
  };

  enum FontAsset
  {
    FONT_ASSET_ELEMENT = 0,
    FONT_ASSET_ELEMENT_BIG,
    FONT_ASSET_ELEMENT_SMALL,
    FONT_ASSET_HELPPANEL_BIG,
    FONT_ASSET_HELPPANEL_SMALL,
    FONT_ASSET_LOGGER,
    FONT_ASSET_LABEL,
    FONT_ASSET_BUTTON_SMALL,
    FONT_ASSET_BUTTON_MEDIUM,
    FONT_ASSET_BUTTON = FONT_ASSET_BUTTON_MEDIUM,
    FONT_ASSET_BUTTON_BIG,
    FONT_ASSET_DEFAULT_FIXED,
    FONT_ASSET_DEFAULT_PROPORTIONAL,

    // MUST REMAIN LAST
    FONT_ASSET_NONE,
    FONT_ASSET_COUNT=FONT_ASSET_NONE
  };

  const u32 ZSHEET_IMAGE_HEIGHT = 2200;

  const u32 ZSHEET_HEIGHTS[] = {
    200, 180, 162, 145, 130, 117, 105, 94, 84, 75, 67, 60,
    54, 48, 43, 38, 34, 30, 27, 24, 21, 19, 17, 15, 
    13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
  };

  enum { ZSHEET_HEIGHT_COUNT = sizeof(ZSHEET_HEIGHTS) / sizeof (ZSHEET_HEIGHTS[0]) };

  const u32 ZSHEET_START_ROWS[ZSHEET_HEIGHT_COUNT] = {
    0,
    0+200, 
    0+200+180, 
    0+200+180+162,
    0+200+180+162+145,
    0+200+180+162+145+130,
    0+200+180+162+145+130+117,
    0+200+180+162+145+130+117+105,
    0+200+180+162+145+130+117+105+94,
    0+200+180+162+145+130+117+105+94+84,
    0+200+180+162+145+130+117+105+94+84+75,
    0+200+180+162+145+130+117+105+94+84+75+67,
    0+200+180+162+145+130+117+105+94+84+75+67+60,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7+6,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7+6+5,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7+6+5+4,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7+6+5+4+3,
    0+200+180+162+145+130+117+105+94+84+75+67+60+54+48+43+38+34+30+27+24+21+19+17+15+13+11+10+9+8+7+6+5+4+3+2
  };

  enum MasterIconZSheetSlot
  {
    ZSLOT_TRANSPORT_OLDEST = 0,
    ZSLOT_TRANSPORT_REWIND,
    ZSLOT_TRANSPORT_BACK,
    ZSLOT_TRANSPORT_STOP,
    ZSLOT_TRANSPORT_STEP,
    ZSLOT_TRANSPORT_FF,
    ZSLOT_TRANSPORT_NEWEST,
    ZSLOT_GRIDTOOL_PENCIL,
    ZSLOT_GRIDTOOL_ERASER,
    ZSLOT_GRIDTOOL_AIRBRUSH,
    ZSLOT_GRIDTOOL_XRAY,
    ZSLOT_GRIDTOOL_BUCKET,
    ZSLOT_GRIDTOOL_CLONE,
    ZSLOT_ICON_ERROR,
    ZSLOT_ICON_CHECKBOX_OFF,
    ZSLOT_ICON_CHECKBOX_ON,
    ZSLOT_TOOLSHAPE_ROUND,
    ZSLOT_TOOLSHAPE_SQUARE,
    ZSLOT_TOOLSHAPE_DIAMOND,
    ZSLOT_GRIDTOOL_SPARK,
    ZSLOT_GRIDTOOL_ATOM_SELECT,
    ZSLOT_GRIDTOOL_TILE_SELECT,
    ZSLOT_SLIDER_HANDLE,

    ZSLOT_NONE = U32_MAX
  };

  struct IconAsset
  {
    static u32 FindZSheetRowIndex(s32 forSize)
    {
      s32 loidx = 0;
      s32 hiidx = ZSHEET_HEIGHT_COUNT-1;
      //      if (forSize >= ZSHEET_HEIGHTS[0]) return 0; // That's all we've got
      do {
        s32 halfidx = (hiidx - loidx) / 2 + loidx;
        s32 midsize = ZSHEET_HEIGHTS[halfidx];
        if (forSize == midsize) return (u32) halfidx;
        if (forSize > midsize) 
          hiidx = halfidx - 1;                 
        else
          loidx = halfidx + 1;
      } while (hiidx >= loidx);
      // loidx passed hiidx, into a _smaller_ size
      // which is what we want
      return (u32) loidx;
    }

    IconAsset(ImageAsset ia = IMAGE_ASSET_MASTER_ICON_ZSHEET, u32 iconSlot = ZSLOT_NONE)
      : m_imageAsset(ia)
      , m_iconSlot(iconSlot)
      , m_enabled(true)
    { }

    void SetImageAsset(ImageAsset ia)
    {
      m_imageAsset = ia;
    }

    ImageAsset GetImageAsset() const
    {
      return m_imageAsset;
    }

    void SetEnabled(bool isEnabled)
    {
      m_enabled = isEnabled;
    }

    bool IsEnabled() const
    {
      return m_enabled;
    }

    void SetIconSlot(u32 slot)
    {
      m_iconSlot = slot;
    }

    bool HasImageAsset() const { 
      return GetImageAsset() != IMAGE_ASSET_NONE;
    }

    bool HasIcon() const {
      return HasImageAsset() && GetIconSlot() != ZSLOT_NONE;
    }

    u32 GetIconSlot() const
    {
      return m_iconSlot;
    }

    ImageAsset m_imageAsset;
    u32 m_iconSlot;
    bool m_enabled;
  };



  class AssetManager
  {
  private:

    static SDL_Surface* surfaces[IMAGE_ASSET_COUNT];

    static TTF_Font* fonts[FONT_ASSET_COUNT];

    static bool initialized;

    static SDL_Surface* LoadImage(const char* relativeFilename)
    {
      OString512 path;
      SDL_Surface* loaded = NULL;
      SDL_Surface* opped = NULL;

      if(Utils::GetReadableResourceFile(relativeFilename, path) && !path.HasOverflowed())
      {
        const char * filename = path.GetZString();
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
      OString512 filename;
      TTF_Font* font = NULL;

      if(Utils::GetReadableResourceFile(relativePath, filename))
      {
        const char * path = filename.GetZString();
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

    static bool IsInitialized()
    {
      return initialized;
    }

    /**
     * Initializes all held Assets. This should only be called once a
     * screen has been created by SDL_SetVideoMode .
     */
    static void Initialize()
    {
      if(!initialized)
      {
        surfaces[IMAGE_ASSET_MASTER_ICON_ZSHEET] = LoadImage("images/mfms-icons-ZSHEET.png");

        const char * FIX_FONT = "fonts/Inconsolata.ttf";
        //        const char * PRO_FONT = "fonts/Mate-Regular.ttf";
        const char * PSC_FONT = "fonts/MateSC-Regular.ttf";

        fonts[FONT_ASSET_ELEMENT] = LoadFont(FIX_FONT, 26);
        fonts[FONT_ASSET_ELEMENT_BIG] = LoadFont(FIX_FONT, 40);
        fonts[FONT_ASSET_ELEMENT_SMALL] = LoadFont(FIX_FONT, 22);
        fonts[FONT_ASSET_HELPPANEL_BIG] = LoadFont(FIX_FONT, 26);
        fonts[FONT_ASSET_HELPPANEL_SMALL] = LoadFont(FIX_FONT, 20);
        fonts[FONT_ASSET_LOGGER] = LoadFont(FIX_FONT, 14);
        fonts[FONT_ASSET_LABEL] = LoadFont(PSC_FONT, 20);
        fonts[FONT_ASSET_BUTTON_SMALL] = LoadFont(PSC_FONT, 16);
        fonts[FONT_ASSET_BUTTON_MEDIUM] = LoadFont(PSC_FONT, 20);
        fonts[FONT_ASSET_BUTTON_BIG] = LoadFont(PSC_FONT, 24);

        fonts[FONT_ASSET_DEFAULT_FIXED] = LoadFont(FIX_FONT, 24);
        fonts[FONT_ASSET_DEFAULT_PROPORTIONAL] = LoadFont(PSC_FONT, 24);

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
        for(ImageAsset i = (ImageAsset) 0; i < IMAGE_ASSET_COUNT; i = (ImageAsset) (i + 1))
        {
          SDL_FreeSurface(surfaces[i]);
          surfaces[i] = NULL;
        }

        for(FontAsset i = (FontAsset) 0; i < FONT_ASSET_COUNT; i = (FontAsset) (i + 1))
        {
          TTF_CloseFont(fonts[i]);
          fonts[i] = NULL;
        }

        initialized = false;
      }
    }

    static UPoint GetSize(ImageAsset a)
    {
      SDL_Surface * s = GetReal(a);
      return UPoint(s->w, s->h);
    }

    static SDL_Surface* Get(ImageAsset a)
    {
      if (!initialized) FAIL(ILLEGAL_STATE);
      if (a >= IMAGE_ASSET_NONE) return 0;
      return surfaces[a];
    }

    static SDL_Surface* GetReal(ImageAsset a)
    {
      SDL_Surface* s = Get(a);
      if (!s) FAIL(ILLEGAL_ARGUMENT);
      return s;
    }

    static TTF_Font* Get(FontAsset a)
    {
      if (!initialized) FAIL(ILLEGAL_STATE);
      if (a >= FONT_ASSET_NONE) return 0;
      return fonts[a];
    }

    static TTF_Font* GetReal(FontAsset a)
    {
      TTF_Font * f = Get(a);
      if (!f) FAIL(ILLEGAL_ARGUMENT);
      return f;
    }

    /**
       Get the 'line skip' -- the recommended interline spacing -- of
       font asset \c a

       \fails ILLEGAL_ARGUMENT if there is no font associated with \c a
     */
    static u32 GetFontLineSkip(FontAsset a)
    {
      TTF_Font * f = GetReal(a);
      return TTF_FontLineSkip(f);
    }

    static UPoint GetFontTextSize(FontAsset font, const char * text)
    {
      MFM_API_ASSERT_NONNULL(text);
      TTF_Font * ttfont = GetReal(font);
      s32 width, height;
      if (TTF_SizeText(ttfont, text, &width, &height) != 0)
      {
        width = height = 0;
      }
      return MakeUnsigned(SPoint(width,height));
    }
  };
}

#endif /* ASSETMANAGER_H */
