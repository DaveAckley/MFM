#include "RectIterator.h"

namespace MFM {
  RectIterator::RectIterator()
    : mRect()
    , mSize()
    , mCurr(0,0)
  { }

  void RectIterator::begin(const Rect rect) {
    mRect = rect;
    mSize = mRect.GetSize();
    mCurr = UPoint(0,0);
  }

  bool RectIterator::hasNext() const {
    return mCurr.GetX() < mSize.GetX()-1 || mCurr.GetY() < mSize.GetY()-1;
  }

  SPoint RectIterator::next() {
    if (!hasNext()) FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    UPoint cur = mCurr;
    mCurr.SetX(1+mCurr.GetX());
    if (mCurr.GetX() >= mSize.GetX()) {
      mCurr.SetX(0);
      mCurr.SetY(1+mCurr.GetY());
    }
    return mRect.GetPosition()+MakeSigned(cur);
  }
}
