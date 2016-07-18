#include <math.h>     /* For floor, sqrt */
#include "Drawing.h"
#include "Util.h"     /* For MIN */

namespace MFM
{

  void Drawing::Convert(const Rect & rect, SDL_Rect & toFill)
  {
    toFill.x = rect.GetX();
    toFill.y = rect.GetY();
    toFill.w = rect.GetWidth();
    toFill.h = rect.GetHeight();
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

  void Drawing::SetWindow(const Rect & rect)
  {
    m_rect = rect;
  }

  void Drawing::GetWindow(Rect & rect) const
  {
    rect = m_rect;
  }

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
  }

  void Drawing::Clear()
  {
    FillRect(0, 0, m_rect.GetWidth(), m_rect.GetHeight(), m_bgColor);
  }

  void Drawing::DrawHLine(int y, int startX, int endX, u32 color) const
  {
    FillRect(startX, y, endX-startX, 1, color);
  }

  void Drawing::DrawVLine(int x, int startY, int endY, u32 color) const
  {
    FillRect(x, startY, 1, endY-startY, color);
  }

  void Drawing::DrawMaskedLineDitColor(int x1, int y1, int x2, int y2, u32 color, u32 mask) const
  {

    // Bresenham's line algorithm, via stackoverflow and Rosetta code, ditified and dashified
    const bool steep = (ABS(y2 - y1) > ABS(x2 - x1));
    if(steep)
    {
      SWAP(x1, y1);
      SWAP(x2, y2);
    }

    if(x1 > x2)
    {
      SWAP(x1, x2);
      SWAP(y1, y2);
    }

    const int dx = x2 - x1;
    const int dy = ABS(y2 - y1);

    const int INCR_DITS = DIT_PER_PIX;
    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? INCR_DITS : -INCR_DITS;
    int y = (int)y1;

    const int maxX = (int)x2;

    for(int x=(int) x1; x<maxX; x += INCR_DITS, mask = (mask<<1) | (mask>>31)) 
    {
      if (mask & (1<<31)) {
        if(steep) FillRectDit(y, x, INCR_DITS, INCR_DITS, color);
        else      FillRectDit(x, y, INCR_DITS, INCR_DITS, color);
      }

      error -= dy;
      if(error < 0)
      {
        y += ystep;
        error += dx;
      }
    }
  }

  void Drawing::DrawRectangle(const Rect & rect) const
  {
    DrawHLine(rect.GetY(),rect.GetX(),rect.GetX()+rect.GetWidth());
    DrawHLine(rect.GetY()+rect.GetHeight()-1,rect.GetX(),rect.GetX()+rect.GetWidth());

    DrawVLine(rect.GetX(),rect.GetY(),rect.GetY()+rect.GetHeight());
    DrawVLine(rect.GetX()+rect.GetWidth()-1,rect.GetY(),rect.GetY()+rect.GetHeight());
  }

  void Drawing::DrawRectDit(const Rect & rect) const
  {
    DrawHLineDit(rect.GetY(),rect.GetX(),rect.GetX()+rect.GetWidth());
    DrawHLineDit(rect.GetY()+rect.GetHeight()-1,rect.GetX(),rect.GetX()+rect.GetWidth());

    DrawVLineDit(rect.GetX(),rect.GetY(),rect.GetY()+rect.GetHeight());
    DrawVLineDit(rect.GetX()+rect.GetWidth()-1,rect.GetY(),rect.GetY()+rect.GetHeight());
  }

  void Drawing::FillRect(int x, int y, int w, int h) const
  {
    FillRect(x, y, w, h, m_fgColor);
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

  void Drawing::FillRectDit(int x, int y, int w, int h, u32 color) const
  {
    FillRect(MapDitToPix(x),
             MapDitToPix(y),
             MapDitToPixCeiling(w),
             MapDitToPixCeiling(h),
             color);
  }

  void Drawing::FillRectDit(const Rect & r, u32 color) const
  {
    FillRectDit(r.GetX(), r.GetY(), r.GetWidth(), r.GetHeight(), color);
  }

  void Drawing::FillCircleDit(const Rect & r, u32 radiusdit, u32 color) const
  {
    int rad = (int) MapPixToDit(MapDitToPix(radiusdit));
    double cxdit = r.GetX()+r.GetWidth()/2.0;
    double cydit = r.GetY()+r.GetHeight()/2.0;
    for(int dydit = 0; dydit <= rad; dydit += DIT_PER_PIX)
    {
      double dxdit = floor(sqrt(2.0*rad*dydit - 1.0*dydit*dydit));
      DrawHLineDit((int) (cydit + rad - dydit), (int) (cxdit-dxdit), (int)(cxdit+dxdit), color);
      DrawHLineDit((int) (cydit - rad + dydit), (int) (cxdit-dxdit), (int)(cxdit+dxdit), color);
    }
  }


  void Drawing::FillCircle(int x, int y, int w, int h, int radius) const
  {
    double cx = x+w/2.0;
    double cy = y+h/2.0;
    for(int dy = 1; dy <= radius; dy++)
    {
      double dx = floor(sqrt(2.0*radius*dy - dy*dy));
      DrawHLine((int) (cy + radius - dy), (int) (cx-dx), (int)(cx+dx));
      DrawHLine((int) (cy - radius + dy), (int) (cx-dx), (int)(cx+dx));
    }
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

  void Drawing::BlitImageAssetTiled(ImageAsset asset, const Rect & destRegion) const
  {
    if (asset != IMAGE_ASSET_NONE)
      BlitImageTiled(AssetManager::GetReal(asset), destRegion);
  }

  void Drawing::BlitText(const char* message, SPoint loc, UPoint size) const
  {
    TTF_Font * ttfont = AssetManager::GetReal(m_fontAsset);

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

  static const SPoint backingPts[4] =
  {
    SPoint(1,1), SPoint(-1,-1), SPoint(1,-1), SPoint(-1,1)
  };

  SPoint Drawing::GetTextSize(const char* message)
  {
    return GetTextSizeInFont(message, m_fontAsset);
  }

  SPoint Drawing::GetTextSizeInFont(const char* message, FontAsset font)
  {
    TTF_Font * ttfont = AssetManager::Get(font);

    s32 w = -1;
    s32 h = -1;
    if (ttfont)
    {
      if (TTF_SizeText(ttfont, message, &w, &h) != 0)
      {
        w = h = -1;  // Might TTF_SizeText have messed with them?
      }
    }
    return SPoint(w,h);
  }

  void Drawing::BlitBackedTextCentered(const char* message, SPoint loc, UPoint size)
  {
    SPoint ssize = MakeSigned(size);
    SPoint tsize = GetTextSize(message);
    if (tsize.GetX() < 0 || tsize.GetY() < 0)
    {
      tsize = SPoint(0,0);  // WTF?
    }

    // Extra subtraction because blit backing makes text 1 pixel bigger all around
    BlitBackedText(message, loc + ssize / 2 - tsize / 2 - SPoint(1, 1), size);
  }

  void Drawing::BlitBackedText(const char* message, SPoint loc, UPoint size)
  {
    u32 oldFG = GetForeground();
    SPoint backingPt;

    loc.Set(loc.GetX() + 1, loc.GetY() + 1);  /* Since we are backing, add more room. */
    SetForeground(GetBackground());

    for(u32 i = 0; i < 4; i++)
    {
      backingPt.Set(loc.GetX() + backingPts[i].GetX(), loc.GetY() + backingPts[i].GetY());
      BlitText(message, backingPt, size);
    }

    SetForeground(oldFG);
    BlitText(message, loc, size);
  }
} /* namespace MFM */
