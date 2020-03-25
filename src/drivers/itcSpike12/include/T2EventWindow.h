/* -*- C++ -*- */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

#include <assert.h>

// Spike files
#include "T2Tile.h"
#include "EWSet.h"

namespace MFM {

  enum T2EventWindowStatus {
     UNKNOWN,
     FREE,
     ACTIVE,
     PASSIVE
  };

  typedef u8 EWSlotNum;
  
  struct T2EventWindow : public EWLinks {

    virtual T2EventWindow * asEventWindow() { return this; }

    T2EventWindow(T2Tile& tile, EWSlotNum ewsn)
      : mTile(tile)
      , mSlotNum(ewsn)
      , mStatus(UNKNOWN)
    { }

    virtual ~T2EventWindow() { }

    T2Tile & mTile;
    const EWSlotNum mSlotNum;

    T2EventWindowStatus status() const { return mStatus; }
    EWSlotNum slotNum() const { return mSlotNum; }
  private:
    T2EventWindowStatus mStatus;
    void setStatus(T2EventWindowStatus ews) { mStatus = ews; }
    friend class EWSet;
  };
}
#endif /* EVENTWINDOW_H */
