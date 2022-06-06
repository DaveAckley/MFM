/*                                              -*- mode:C++ -*-
  WrappedText.h Static text with line-breaking
  Copyright (C) 2021 Living Computation Foundation.  All rights reserved.

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
  \file WrappedText.h Static text with line-breaking
  \author David H. Ackley.
  \date (C) 2021 All rights reserved.
  \lgpl
 */
#ifndef WRAPPEDTEXT_H
#define WRAPPEDTEXT_H

#include <string>
#include <list>

#include "itype.h"
#include "SDL_ttf.h"
#include "Point.h"
#include "TextPanel.h"
#include "Drawing.h"
#include "Fail.h"

namespace MFM
{
  /**
   * A background-less Panel which displays some possibly multiline
   * text, broken on whitespace if possible
   */
  class WrappedText : public GenericTextPanel {
  public:
    typedef MovablePanel Super;

    virtual ResettableByteSink & GetByteSink() {
      FAIL(UNSUPPORTED_OPERATION);
    }

    // Wrapped text doesn't handle events
    virtual bool Handle(KeyboardEvent& event) {
      return false;
    }

    // I said.. Wrapped text doesn't handle events
    virtual bool Handle(MouseButtonEvent& event) {
      return false;
    }

    /** No border */
    virtual void PaintBorder(Drawing & draw) { }

    /** Draw the text */
    virtual void PaintComponent(Drawing & draw) ;

    /** Reinitialize to empty */
    void Init() ;
    /**
     * Default text color
     */
    static const u32 WRAPPEDTEXT_COLOR = Drawing::GREY05;

    WrappedText();

    virtual ~WrappedText();

    void SetText(std::string text) ;

  private:
    struct Segment {
      SPoint mStart;
      u32 mWhiteWidth;
      UPoint mBlackSize;
      std::string mWhite;
      std::string mBlack;
      Segment(std::string white, std::string black)
        : mStart(0,0)
        , mWhiteWidth(0)
        , mBlackSize(0,0)
        , mWhite(white)
        , mBlack(black)
      { }
    };
    typedef std::list<Segment> Segments;
    Segments mSegments;
    std::string mFullText;
    TTF_Font * mZFont;
    UPoint mDimensions;

    void ReflowText() ;

    void drawSegment(Drawing& drawing, const Segment & segment) const ;

    static UPoint getTextSize(TTF_Font * ttfont, const char * text) ;


  };
}

#endif /* WRAPPEDTEXT_H */
