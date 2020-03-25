#include "EWSet.h"

#include "T2Tile.h"
#include "T2EventWindow.h"

namespace MFM {
  bool EWSet::isEmpty() const { return !isLinked(); }

  void EWSet::push(T2EventWindow* ew) {
    assert(ew && !ew->isLinked());
    ew->linkAfter(this);
  }

  void EWSet::pushBack(T2EventWindow* ew) {
    assert(ew && !ew->isLinked());
    ew->linkBefore(this);
  }

  T2EventWindow * EWSet::pop() {
    if (!isLinked()) return 0;
    T2EventWindow * ret = mNext->asEventWindow();
    assert(ret);
    ret->unlink();
    return ret;
  }

  EWSet::~EWSet() {
    T2EventWindow * ew;
    while ((ew = pop()) != 0) {
      delete ew;
    }
  }
}
