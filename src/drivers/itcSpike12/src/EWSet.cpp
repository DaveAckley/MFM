#include "EWSet.h"

#include "T2Tile.h"
#include "T2EventWindow.h"

namespace MFM {
  void EWLinks::insert(EWSet * ews) {
    assert(ews != 0);
    assert(mInSet == 0);
    ews->rawInsert(this);
  }

  bool EWLinks::remove() {
    if (mInSet == 0) {
      assert(mIdxInSet == U32_MAX);
      return false;
    } 
    mInSet->rawRemove(this);
    return true;
  }

  bool EWSet::isEmpty() const {
    return mInUse == 0;
  }

  void EWSet::rawInsert(EWLinks * l) {
    assert(l !=0 && !l->isInSet());
    if (mMembers.size() == mInUse)
      mMembers.push_back(l);
    else
      mMembers[mInUse] = l;
    l->mInSet = this;
    l->mIdxInSet = mInUse++;
  }

  void EWSet::rawRemove(EWLinks * l) {
    assert(l != 0 && l->inSet() == this);
    u32 idx = l->getIndexInSet();
    assert(idx < mInUse);
    l->mInSet = 0;
    l->mIdxInSet = U32_MAX;
    --mInUse;
    if (mInUse > 0 && idx != mInUse) {
      mMembers[idx] = mMembers[mInUse];
      mMembers[idx]->mIdxInSet = idx;
    }
  }

  EWLinks * EWSet::removeRandom() {
    if (mInUse == 0) return 0;
    Random & r = mTile.getRandom();
    EWLinks * ret = mMembers[r.Between(0,mInUse-1)];
    ret->remove();
    return ret;
  }

  EWSet::~EWSet() {
    // EWSet doesn't own its contents, but let's complain if our
    // owners didn't tidy up before axing us
    if (!isEmpty()) {
      fprintf(stderr,"~EWSet: NON EMPTY\n");
    }
  }
}

