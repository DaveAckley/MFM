/* -*- C++ -*- */
#ifndef RECTITERATOR_H
#define RECTITERATOR_H

#include "Rect.h"

namespace MFM {
  struct RectIterator {
    RectIterator() ;
    Rect mRect;
    UPoint mSize;
    UPoint mCurr;

    const Rect & getRect() const { return mRect; }
    void begin(const Rect rect) ;
    bool hasNext() const ;
    SPoint next() ;
  };

}
#endif /* RECTITERATOR_H */
