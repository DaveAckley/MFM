/* -*- C++ -*- */
#ifndef FONTS_H
#define FONTS_H

#include "itype.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "Drawing.h"

namespace MFM
{
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
