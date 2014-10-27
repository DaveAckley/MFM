#include "Fonts.h"
#include "Utils.h"

namespace MFM {

  const char * Fonts::DEFAULT_FONT = "fonts/UbuntuMono-B.ttf";

  const u32 Fonts::DEFAULT_FONT_SIZE = 26;

  void Fonts::Init() {
    if (m_initted) return;

    TTF_Init();
    for (u32 i = 0; i < CACHE_SIZE; ++i) {
      Cache & c = m_fontCache[i];
      c.path = 0;
      c.size = 0;
      c.font = 0;
    }
    m_initted = true;
  }

  TTF_Font * Fonts::GetFont(const char * resourcePath, u32 size)
  {
    FAIL(DEPRECATED);
    Init();
    for (u32 i = 0; i < CACHE_SIZE; ++i)
    {
      Cache & c = m_fontCache[i];
      if (c.path == 0)
      {
        // Not found, set up camp here
        c.path = resourcePath;
        c.size = size;
        const u32 SIZE = 1024;
        char fontloc[SIZE];
        if (!Utils::GetReadableResourceFile(c.path, fontloc, SIZE))
          FAIL(NOT_FOUND);
        c.font = TTF_OpenFont(fontloc, c.size);
        if (!c.font)
          FAIL(ILLEGAL_STATE);
        return c.font;
      }
      if (size == c.size && !strcmp(resourcePath, c.path))
        return c.font;
    }
    FAIL(OUT_OF_ROOM);
  }
}
