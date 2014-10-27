/*                                              -*- mode:C++ -*-
  Fonts.h SDL_Font loading / caching toolkit
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
  \file Fonts.h SDL_Font loading / caching toolkit
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef FONTS_H
#define FONTS_H

#include "itype.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "Drawing.h"

namespace MFM
{
  /**
   * A SDL_Font loading and caching class.
   */
  class Fonts
  {
  private:

    bool m_initted;

    static const u32 CACHE_SIZE = 10;
    struct Cache {
      const char * path;
      u32 size;
      TTF_Font * font;
    } m_fontCache[CACHE_SIZE];

  public:
    static const char * DEFAULT_FONT;
    static const u32 DEFAULT_FONT_SIZE;

    Fonts() : m_initted(false) { }

    void Init();

    TTF_Font * GetFont(const char * resourcePath, u32 size) ;

    TTF_Font * GetDefaultFont(u32 size = DEFAULT_FONT_SIZE) {
      return GetFont(DEFAULT_FONT, size);
    }

  };
}

#endif /* FONTS_H */
