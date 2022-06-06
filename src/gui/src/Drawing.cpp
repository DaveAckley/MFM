#include <math.h>     /* For floor, sqrt */
#include "Drawing.h"
#include "Util.h"     /* For MIN */

namespace MFM
{
  void Drawing::SetSDLColor(SDL_Color & set, const u32 from)
  {
    set.r = (from >> 16) & 0xff;
    set.g = (from >> 8) & 0xff;
    set.b = from & 0xff;
  }

  void Drawing::GetSDLColor(const SDL_Color & from, u32 & to)
  {
    to = (0xff<<24)|(from.r<<16)|(from.g<<8)|from.b;
  }

  Drawing::Drawing(SDL_Surface * dest, FontAsset font)
  {
    Reset(dest,font);
  }

  void Drawing::TransformWindow(const Rect & rect)
  {
    Rect temp;
    TransformWindow(m_rect, rect, temp);
    m_rect = temp;
  }

  void Drawing::TransformWindow(const Rect & existing, const Rect & rect, Rect & result)
  {
    result.SetX(existing.GetX() + rect.GetX());
    result.SetY(existing.GetY() + rect.GetY());
    result.SetWidth(MIN(existing.GetWidth(), rect.GetWidth()));
    result.SetHeight(MIN(existing.GetHeight(), rect.GetHeight()));
  }

  u32 Drawing::SetZoomDits(u32 dits) 
  {
    u32 old = m_drawScaleDits;
    m_drawScaleDits = dits;
    return old;
  }

  u32 Drawing::GetZoomDits() const
  {
    return m_drawScaleDits;
  }

  void Drawing::Reset(SDL_Surface * dest, FontAsset font)
  {
    m_dest = dest;
    m_rect.SetPosition(SPoint());
    if (m_dest) {
      m_rect.SetSize(UPoint(m_dest->w, m_dest->h));
    } else {
      m_rect.SetSize(UPoint(600, 400));
    }

    m_fgColor = YELLOW;
    m_bgColor = BLACK;

    m_fontAsset = font;
    m_drawScaleDits = MapPixToDit(1u);
  }

  void Drawing::FillRect(int x, int y, int w, int h, u32 color) const
  {
    SDL_Rect rect;
    rect.x = x+m_rect.GetX();
    rect.y = y+m_rect.GetY();;
    rect.w = w;
    rect.h = h;

    SDL_Rect clip;
    Convert(m_rect,clip);

    SDL_SetClipRect(m_dest, &clip);
    SDL_FillRect(m_dest, &rect, color);
  }

  static const SPoint backingPts[4] =
  {
    SPoint(1,1), SPoint(-1,-1), SPoint(1,-1), SPoint(-1,1)
  };

  void Drawing::BlitText(const char* message, SPoint loc, UPoint size, TTF_Font * infont) const
  {
    TTF_Font * ttfont = infont;
    if (!ttfont) {
      u32 baseFontSize = AssetManager::GetZFontSize(m_fontAsset); // in pix
      u32 scaledSize = MapDitToPix(m_drawScaleDits*baseFontSize); // in pix
      ttfont = AssetManager::GetZFont(AssetManager::GetZFontPSC(m_fontAsset),scaledSize);
    }

    SDL_Color sdl_color;
    SetSDLColor(sdl_color,m_fgColor);

    SDL_Surface* text = TTF_RenderText_Blended(ttfont, message, sdl_color);

    SDL_Rect rect;
    rect.x = loc.GetX() + m_rect.GetX();
    rect.y = loc.GetY() + m_rect.GetY();
    rect.w = size.GetX();
    rect.h = size.GetY();

    SDL_Rect clip;
    Convert(m_rect,clip);

    SDL_SetClipRect(m_dest, &clip);
    SDL_BlitSurface(text, NULL, m_dest, &rect);

    SDL_FreeSurface(text);
  }

  void Drawing::BlitBackedText(const char* message, SPoint loc, UPoint size, TTF_Font * infont)
  {
    u32 oldFG = GetForeground();
    SPoint backingPt;

    loc.Set(loc.GetX() + 1, loc.GetY() + 1);  /* Since we are backing, add more room. */
    SetForeground(GetBackground());

    for(u32 i = 0; i < 4; i++)
    {
      backingPt.Set(loc.GetX() + backingPts[i].GetX(), loc.GetY() + backingPts[i].GetY());
      BlitText(message, backingPt, size, infont);
    }

    SetForeground(oldFG);
    BlitText(message, loc, size, infont);
  }

  void Drawing::BlitBackedTextCentered(const char* message, SPoint loc, UPoint size, TTF_Font * infont)
  {
    SPoint ssize = MakeSigned(size);
    SPoint tsize = GetTextSize(message, infont);
    if (tsize.GetX() < 0 || tsize.GetY() < 0)
    {
      tsize = SPoint(0,0);  // WTF?
    }

    // Extra subtraction because blit backing makes text 1 pixel bigger all around
    BlitBackedText(message, loc + ssize / 2 - tsize / 2 - SPoint(1, 1), size, infont);
  }

  SPoint Drawing::GetTextSize(const char* message, TTF_Font * infont)
  {
    if (infont) return GetTextSizeInTTFFont(message, *infont);
    return GetTextSizeInFont(message, m_fontAsset);
  }

  SPoint Drawing::GetTextSizeInFont(const char* message, FontAsset font)
  {
    u32 baseFontSize = AssetManager::GetZFontSize(font); // in pix
    u32 scaledSize = MapDitToPix(m_drawScaleDits*baseFontSize); // in pix
    TTF_Font * ttfont = AssetManager::GetZFont(AssetManager::GetZFontPSC(m_fontAsset),scaledSize);
    if (!ttfont) return SPoint(-1,-1);
    return GetTextSizeInTTFFont(message,*ttfont);
  }

  SPoint Drawing::GetTextSizeInTTFFont(const char* message, TTF_Font & font)
  {
    s32 w = -1;
    s32 h = -1;
    if (TTF_SizeText(&font, message, &w, &h) != 0)
    {
      w = h = -1;  // Might TTF_SizeText have messed with them?
    }
    return SPoint(w,h);
  }

  void Drawing::BlitImage(SDL_Surface* src, SPoint loc, UPoint maxSize) const
  {
    if(!src)
    {
      FAIL(ILLEGAL_STATE);
    }

    SDL_Rect rect;
    rect.x = loc.GetX() + m_rect.GetX();
    rect.y = loc.GetY() + m_rect.GetY();
    rect.w = maxSize.GetX();
    rect.h = maxSize.GetY();

    SDL_Rect clip;
    Convert(m_rect, clip);

    SDL_SetClipRect(m_dest, &clip);
    SDL_BlitSurface(src, NULL, m_dest, &rect);
  }

  void Drawing::BlitImageTiled(SDL_Surface* src, const Rect & destRegion) const
  {
    if(!src)
    {
      FAIL(ILLEGAL_STATE);
    }

    // Don't draw outside destregion or existing clip
    Rect target = destRegion;
    target &= m_rect;

    SDL_Rect clip;
    Convert(target, clip);

    SDL_SetClipRect(m_dest, &clip);

    SDL_Rect rect;
    rect.w = src->w;
    rect.h = src->h;

    for (s32 y = 0; y < (s32) destRegion.GetHeight(); y += (s32) src->h)
    {
      for (s32 x = 0; x < (s32) destRegion.GetWidth(); x += (s32) src->w)
      {
        rect.x = destRegion.GetX() + x;
        rect.y = destRegion.GetY() + y;
        SDL_BlitSurface(src, NULL, m_dest, &rect);
      }
    }
  }

  void Drawing::BlitImageAsset(ImageAsset asset, SPoint loc, UPoint maxSize) const
  {
    if (asset != IMAGE_ASSET_NONE)
      BlitImage(AssetManager::GetReal(asset), loc, maxSize);
  }

  void Drawing::BlitImageAsset(ImageAsset asset, SPoint loc) const
  {
    if (asset == IMAGE_ASSET_NONE) return;
    SDL_Surface* s= AssetManager::GetReal(asset);
    UPoint maxsize(s->w, s->h);
    BlitImage(s, loc, maxsize);
  }

  void Drawing::BlitImageAssetTiled(ImageAsset asset, const Rect & destRegion) const
  {
    if (asset != IMAGE_ASSET_NONE)
      BlitImageTiled(AssetManager::GetReal(asset), destRegion);
  }

  void Drawing::BlitSubImage(SDL_Surface* src, const Rect & srcRegion, SPoint destLoc) const
  {
    if(!src)
    {
      FAIL(ILLEGAL_STATE);
    }

    // Get size from src region
    Rect target = srcRegion;

    // And position from dest, mapped to screen coordinates
    target.SetPosition(destLoc + m_rect.GetPosition());

    // Clip against current drawing window
    target &= m_rect;

    SDL_Rect clip;
    Convert(target, clip);

    SDL_Rect srcClip;
    Convert(srcRegion, srcClip);

    SDL_SetClipRect(m_dest, &clip);
    SDL_BlitSurface(src, &srcClip, m_dest, &clip);
  }

  void Drawing::BlitIconAsset(const IconAsset & iconasset, u32 atSize, SPoint destLoc) const
  {
    if (!iconasset.HasIcon()) return;
    ImageAsset ia = iconasset.GetImageAsset();

    SDL_Surface* s= AssetManager::GetReal(ia);

    u32 slot = iconasset.GetIconSlot();
    u32 idx = IconAsset::FindZSheetRowIndex(atSize);
    u32 height = ZSHEET_HEIGHTS[idx];
    SPoint pos;
    if (iconasset.IsEnabled()) 
    {
      pos.Set(height * slot, ZSHEET_IMAGE_HEIGHT - ZSHEET_START_ROWS[idx] - height);
    }
    else
    {
      u32 icons = s->w/200;
      u32 xstart = s->w - icons*height;
      pos.Set(xstart + height * slot, ZSHEET_START_ROWS[idx]);
    }
    Rect srcRect(pos, UPoint(height, height));
    BlitSubImage(s, srcRect, destLoc);
  }

  void Drawing::Convert(const Rect & rect, SDL_Rect & toFill)
  {
    toFill.x = rect.GetX();
    toFill.y = rect.GetY();
    toFill.w = rect.GetWidth();
    toFill.h = rect.GetHeight();
  }

} /* namespace MFM */
