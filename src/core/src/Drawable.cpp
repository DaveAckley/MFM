#include <math.h>     /* For floor, sqrt */
#include "Drawable.h"
#include "Util.h"     /* For MIN */

namespace MFM
{

  void Drawable::SetWindow(const Rect & rect)
  {
    m_rect = rect;
  }

  void Drawable::GetWindow(Rect & rect) const
  {
    rect = m_rect;
  }

  Drawable::Drawable()
  {
    Reset();
  }

  void Drawable::Reset()
  {
    m_rect.SetPosition(SPoint());
    m_rect.SetSize(UPoint(600, 400));

    m_fgColor = YELLOW;
    m_bgColor = BLACK;

    m_drawScaleDits = MapPixToDit(1u);
    m_ditsPerSite = MapPixToDit(1u);
  }

  void Drawable::Clear()
  {
    FillRect(0, 0, m_rect.GetWidth(), m_rect.GetHeight(), m_bgColor);
  }

  void Drawable::DrawHLine(int y, int startX, int endX, u32 color) const
  {
    FillRect(startX, y, endX-startX, 1, color);
  }

  void Drawable::DrawVLine(int x, int startY, int endY, u32 color) const
  {
    FillRect(x, startY, 1, endY-startY, color);
  }

  void Drawable::DrawScaledMaskedLineDitColor(int x1, int y1, int x2, int y2, u32 onColor, u32 offColor, u32 mask, u32 maskDit, u32 widthDits) const
  {
    bool hasOffColor = onColor != offColor;
    u32 halfWidthDits = widthDits/2;

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

    int maskDitsLeft = 0;
    for(int x=(int) x1; x<maxX; x += INCR_DITS) 
    {
      if ((maskDitsLeft -= INCR_DITS) <= 0) {
        mask = (mask<<1) | (mask>>31);
        maskDitsLeft += maskDit;
      }

      bool thisOn = mask & (1<<31);
      if (hasOffColor || thisOn) {
        const u32 thisColor = thisOn ? onColor : offColor;
        if(steep) FillRectDit(y-halfWidthDits, x, widthDits, INCR_DITS, thisColor);
        else      FillRectDit(x, y-halfWidthDits, INCR_DITS, widthDits, thisColor);
      }

      error -= dy;
      if(error < 0)
      {
        y += ystep;
        error += dx;
      }
    }
  }

  void Drawable::DrawRectangle(const Rect & rect) const
  {
    DrawHLine(rect.GetY(),rect.GetX(),rect.GetX()+rect.GetWidth());
    DrawHLine(rect.GetY()+rect.GetHeight()-1,rect.GetX(),rect.GetX()+rect.GetWidth());

    DrawVLine(rect.GetX(),rect.GetY(),rect.GetY()+rect.GetHeight());
    DrawVLine(rect.GetX()+rect.GetWidth()-1,rect.GetY(),rect.GetY()+rect.GetHeight());
  }

  void Drawable::DrawRectDit(const Rect & rect) const
  {
    DrawHLineDit(rect.GetY(),rect.GetX(),rect.GetX()+rect.GetWidth());
    DrawHLineDit(rect.GetY()+rect.GetHeight()-1,rect.GetX(),rect.GetX()+rect.GetWidth());

    DrawVLineDit(rect.GetX(),rect.GetY(),rect.GetY()+rect.GetHeight());
    DrawVLineDit(rect.GetX()+rect.GetWidth()-1,rect.GetY(),rect.GetY()+rect.GetHeight());
  }

  void Drawable::FillRect(int x, int y, int w, int h) const
  {
    FillRect(x, y, w, h, m_fgColor);
  }

  void Drawable::FillRectDit(int x, int y, int w, int h, u32 color) const
  {
    FillRect(MapDitToPix(x),
             MapDitToPix(y),
             MapDitToPixCeiling(w),
             MapDitToPixCeiling(h),
             color);
  }

  void Drawable::FillRectDit(const Rect & r, u32 color) const
  {
    FillRectDit(r.GetX(), r.GetY(), r.GetWidth(), r.GetHeight(), color);
  }

  void Drawable::FillCircleDit(const Rect & r, u32 radiusdit, u32 color) const
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


  void Drawable::FillCircle(int x, int y, int w, int h, int radius) const
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

} /* namespace MFM */
