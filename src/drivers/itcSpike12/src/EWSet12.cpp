#include "EWSet12.h"

#include "Tile12.h"
#include "EventWindow12.h"

namespace MFM {
  bool EWSet::isEmpty() const { return !isLinked(); }

  void EWSet::push(EventWindow* ew) {
    assert(ew && !ew->isLinked());
    ew->linkAfter(this);
  }

  void EWSet::pushBack(EventWindow* ew) {
    assert(ew && !ew->isLinked());
    ew->linkBefore(this);
  }

  EventWindow * EWSet::pop() {
    if (!isLinked()) return 0;
    EventWindow * ret = mNext->asEventWindow();
    assert(ret);
    ret->unlink();
    return ret;
  }

  EWSet::~EWSet() {
    EventWindow * ew;
    while ((ew = pop()) != 0) {
      delete ew;
    }
  }
}
