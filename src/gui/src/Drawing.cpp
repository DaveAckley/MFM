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

  u32 Drawing::InterpolateColors(const u32 color1, const u32 color2, const u32 percentOfColor1)
  {
    if (percentOfColor1 >= 100) return color1;
    if (percentOfColor1 == 0) return color2;

    const u32 percentOfColor2 = 100 - percentOfColor1;
    u32 res = 0;
    for (u32 i = 0; i < 32; i += 8) {
      u32 comp1 = (color1 >> i) & 0xff;
      u32 comp2 = (color2 >> i) & 0xff;
      u32 mix = (percentOfColor1 * comp1 + percentOfColor2 * comp2) / 100;
      res |= mix << i;
    }

    return res;
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

  Drawing::Drawing(SDL_Surface * dest, TTF_Font * font)
  {
    Reset(dest,font);
  }

  void Drawing::Reset(SDL_Surface * dest, TTF_Font * font)
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

    m_font = font;
  }

  void Drawing::Clear()
  {
    FillRect(0, 0, m_rect.GetWidth(), m_rect.GetHeight(), m_bgColor);
  }

  void Drawing::DrawHLine(int y, int startX, int endX) const
  {
    FillRect(startX,y,endX-startX,1);
  }

  void Drawing::DrawVLine(int x, int startY, int endY) const
  {
    FillRect(x,startY,1,endY-startY);
  }

  void Drawing::DrawRectangle(const Rect & rect) const
  {
    DrawHLine(rect.GetY(),rect.GetX(),rect.GetX()+rect.GetWidth());
    DrawHLine(rect.GetY()+rect.GetHeight()-1,rect.GetX(),rect.GetX()+rect.GetWidth());

    DrawVLine(rect.GetX(),rect.GetY(),rect.GetY()+rect.GetHeight());
    DrawVLine(rect.GetX()+rect.GetWidth()-1,rect.GetY(),rect.GetY()+rect.GetHeight());
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

  void Drawing::BlitImage(SDL_Surface* src, UPoint loc, UPoint maxSize) const
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

  void Drawing::BlitAsset(Asset asset, UPoint loc, UPoint maxSize) const
  {
    BlitImage(AssetManager::Get(asset), loc, maxSize);
  }

  void Drawing::BlitText(const char* message, UPoint loc, UPoint size) const
  {
    if (!m_font) FAIL(ILLEGAL_STATE);

    SDL_Color sdl_color;
    SetSDLColor(sdl_color,m_fgColor);

    SDL_Surface* text = TTF_RenderText_Blended(m_font, message, sdl_color);

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
    return GetTextSizeInFont(message, GetFont());
  }

  SPoint Drawing::GetTextSizeInFont(const char* message, TTF_Font * font)
  {
    s32 w = -1;
    s32 h = -1;
    if (font)
    {
      if (TTF_SizeText(font, message, &w, &h) != 0)
      {
        w = h = -1;  // Might TTF_SizeText have messed with them?
      }
    }
    return SPoint(w,h);
  }

  void Drawing::BlitBackedTextCentered(const char* message, UPoint loc, UPoint size)
  {
    SPoint tsize = GetTextSize(message);
    if (tsize.GetX() < 0 || tsize.GetY() < 0)
    {
      tsize = SPoint(0,0);  // WTF?
    }
    UPoint utsize(tsize.GetX(), tsize.GetY());

    // Extra subtraction because blit backing makes text 1 pixel bigger all around
    BlitBackedText(message, loc + size / 2 - utsize / 2 - UPoint(1, 1), size);
  }

  void Drawing::BlitBackedText(const char* message, UPoint loc, UPoint size)
  {
    u32 oldFG = GetForeground();
    SPoint backingPt;

    loc.Set(loc.GetX() + 1, loc.GetY() + 1);  /* Since we are backing, add more room. */
    SetForeground(GetBackground());

    for(u32 i = 0; i < 4; i++)
    {
      backingPt.Set(loc.GetX() + backingPts[i].GetX(), loc.GetY() + backingPts[i].GetY());
      if (CanMakeUnsigned(backingPt))
      {
        BlitText(message, MakeUnsigned(backingPt), size);
      }
    }

    SetForeground(oldFG);
    BlitText(message, loc, size);
  }
} /* namespace MFM */
