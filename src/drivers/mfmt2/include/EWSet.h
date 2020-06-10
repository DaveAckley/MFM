/* -*- C++ -*- */
#ifndef EWSET_H
#define EWSET_H

#include "itype.h"
#include "Dirs.h"

#include <vector>
#include <assert.h>

namespace MFM {

  struct T2EventWindow; // FORWARD
  struct T2Tile; // FORWARD
  struct EWSet;

  struct EWLinks { 

    bool isInSet() const { return mInSet != 0; }

    EWSet * inSet() const { return mInSet; }

    u32 getIndexInSet() const {
      assert(isInSet());
      return mIdxInSet;
    }
    
    virtual T2EventWindow * asEventWindow() { return 0; }

    EWLinks()
      : mInSet(0)
      , mIdxInSet(U32_MAX)
    { }

    virtual ~EWLinks() {
      if (isInSet()) removeFromEWSet();
    }

    void insertInEWSet(EWSet * ews) ;

    bool removeFromEWSet() ;

  private:
    friend EWSet;
    EWSet * mInSet;
    u32 mIdxInSet;
  };

  struct EWSet {
    T2Tile & mTile;
    std::vector<EWLinks*> mMembers;
    u32 mInUse;

    bool isEmpty() const ;
    void rawInsert(EWLinks * l) ;
    void rawRemove(EWLinks * l) ;

    EWLinks * removeRandom() ;

    EWSet(T2Tile& tile)
      : mTile(tile)
      , mInUse(0)
    { }

    ~EWSet() ;  // EWSet DOES NOT own its EWs

  };
}
#endif /* EWSET_H */
