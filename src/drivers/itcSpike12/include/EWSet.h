/* -*- C++ -*- */
#ifndef EWSET_H
#define EWSET_H

#include "itype.h"
#include "Dirs.h"

#include <assert.h>

namespace MFM {

  struct T2EventWindow; // FORWARD
  struct T2Tile; // FORWARD

  struct EWLinks { 
    EWLinks * mNext;
    EWLinks * mPrev;

    bool isLinked() const { return mNext != this; }

    virtual T2EventWindow * asEventWindow() { return 0; }

    EWLinks()
      : mNext(this)
      , mPrev(this)
    { }

    bool unlink() {
      assert(mNext != 0 && mPrev != 0);
      if (mNext == this) {
        assert(mNext == mPrev);
        return false;
      } 
      mPrev->mNext = mNext;
      mNext->mPrev = mPrev;
      mNext = mPrev = this;
      return true;
    }

    void linkAfter(EWLinks * afterThis) {
      assert(afterThis != 0);
      assert(mNext == mPrev && mNext == this);

      this->mNext = afterThis->mNext;
      this->mPrev = afterThis;
      afterThis->mNext->mPrev = this;
      afterThis->mNext = this;
    }

    void linkBefore(EWLinks * beforeThis) {
      assert(beforeThis != 0);
      linkAfter(beforeThis->mPrev);
    }
  };

  struct EWSet : public EWLinks {
    T2Tile & mTile;

    bool isEmpty() const ;
    void push(T2EventWindow* ew) ;
    void pushBack(T2EventWindow* ew) ;
    T2EventWindow * pop() ;

    EWSet(T2Tile& tile)
      : mTile(tile)
    { }

    ~EWSet() ;  // EWSet owns its EWs, must clean them up

  };
}
#endif /* EWSET_H */
