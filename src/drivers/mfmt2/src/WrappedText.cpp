#include "WrappedText.h"
#include "SDL.h"
#include "Drawing.h"

#include <ctype.h>  // For isspace

namespace MFM
{
  void WrappedText::Init() {
    this->SetForeground(WRAPPEDTEXT_COLOR);
    mZFont = this->GetFontReal();
    mFullText = "";
    mSegments.clear();
  }

  WrappedText::WrappedText()
  {
    Init();
  }

  WrappedText::~WrappedText()
  { }

  void WrappedText::PaintComponent(Drawing& drawing)
  {
    drawing.SetForeground(GetForeground());
    drawing.SetBackground(GetBackground());

    UPoint udim = Panel::GetDimensions();
    if (udim != mDimensions) {
      mDimensions = udim;
      ReflowText();
    }
    SPoint dims = MakeSigned(udim);

    for (Segments::const_iterator itr = mSegments.begin(); itr != mSegments.end(); ++itr) {
      drawSegment(drawing,*itr);
    }
  }

  UPoint WrappedText::getTextSize(TTF_Font * ttfont, const char * text) {
    MFM_API_ASSERT_NONNULL(text);
    s32 width, height;
    if (TTF_SizeText(ttfont, text, &width, &height) != 0) {
      width = height = 0;
    }
    return MakeUnsigned(SPoint(width,height));
  }

  void WrappedText::drawSegment(Drawing& drawing, const Segment & segment) const {
    const char * zstr = segment.mBlack.c_str();
    SPoint at(segment.mStart);
    if (at.GetY() < (s32) mDimensions.GetY())
      drawing.BlitText(zstr, at, segment.mBlackSize, mZFont);
  }

  void WrappedText::SetText(std::string text) {
    if (text != mFullText) {
      mFullText = text;
      ReflowText();
    }
  }

  void WrappedText::ReflowText() {
    Segments & segs = mSegments;
    segs.clear();
    mZFont = this->GetFontReal();
    LOG.Message("Reflowing into %dx%d",mDimensions.GetX(), mDimensions.GetY());
    s32 lineskip = TTF_FontLineSkip(mZFont) + m_fontHeightAdjust;

    /*First break the text up into optional-white+non-white segments */
    std::string white = "";
    std::string piece = "";
    bool wantWhite = true;
    for (std::string::iterator itr = mFullText.begin(); itr != mFullText.end(); ++itr) {
      char ch = *itr;
      bool again;
      do {
        again = false;
        if (wantWhite == (bool) isspace(ch)) {
          // Extend existing chunk
          piece += ch;
        } else {
          // Terminate existing chunk
          if (!wantWhite) { // segment completed
            if (white.length() > 0 || piece.length() > 0) {
              segs.push_back(Segment(white,piece));
              white = "";
            }
          } else 
            white = piece; // stash leading white
          piece = "";
          wantWhite = !wantWhite;
          again = true; // Reconsider this byte
        }
      } while (again);
    }
    // Terminate final chunk
    if (white.length() > 0 || piece.length() > 0) {
      if (!wantWhite) segs.push_back(Segment(white,piece));
      else segs.push_back(Segment(piece,""));
    }

    // Size each segment individually
    for (Segments::iterator itr = segs.begin(); itr != segs.end(); ++itr) {
      Segment & seg = *itr;
      seg.mWhiteWidth = getTextSize(mZFont, seg.mWhite.c_str()).GetX();
      seg.mBlackSize = getTextSize(mZFont, seg.mBlack.c_str());
    }

    // Fill lines with white*,nonwhite sequences
    {
      u32 x = 0;
      u32 y = 0;
      u32 w = mDimensions.GetX();
      u32 h = mDimensions.GetY();
      for (Segments::iterator itr = segs.begin(); itr != segs.end(); ++itr) {
        Segment & seg = *itr;
        if (x + seg.mWhiteWidth + seg.mBlackSize.GetX() < w) {
          // Segment fits
          seg.mStart.SetX(x+seg.mWhiteWidth);
          seg.mStart.SetY(y); // this code has variable baseline problems
          x += seg.mWhiteWidth + seg.mBlackSize.GetX();
        } else {
          // Need to break line and discard white on next
          x = 0;
          y += lineskip;
          if (y > h) {
            LOG.Warning("Overflowing box height (%d vs %d)", y, h);
          }
          seg.mStart.SetX(x);
          seg.mStart.SetY(y);
          x += seg.mBlackSize.GetX();
        }
      }
    }

    // SHOW SEGS
    for (Segments::iterator itr = segs.begin(); itr != segs.end(); ++itr) {
      const Segment & seg = *itr;
      LOG.Message("SEG white '%s' %d + black '%s' %dx%d@(%d,%d)",
                  seg.mWhite.c_str(),
                  seg.mWhiteWidth,
                  seg.mBlack.c_str(),
                  seg.mBlackSize.GetX(), seg.mBlackSize.GetY(),
                  seg.mStart.GetX(), seg.mStart.GetY());
    }
    //FAIL(INCOMPLETE_CODE);
  }
}
