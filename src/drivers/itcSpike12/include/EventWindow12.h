/* -*- C++ -*- */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"

#include <assert.h>

// Spike files
#include "Tile12.h"
#include "EWSet12.h"

namespace MFM {

  enum EventWindowStatus {
     UNKNOWN,
     FREE,
     ACTIVE,
     PASSIVE
  };

  typedef u8 EWSlotNum;
  
  struct EventWindow : public EWLinks {

    virtual EventWindow * asEventWindow() { return this; }

    EventWindow(Tile& tile, EWSlotNum ewsn)
      : mTile(tile)
      , mSlotNum(ewsn)
      , mStatus(UNKNOWN)
    { }

    virtual ~EventWindow() { }

    Tile & mTile;
    const EWSlotNum mSlotNum;

    EventWindowStatus status() const { return mStatus; }
    EWSlotNum slotNum() const { return mSlotNum; }
  private:
    EventWindowStatus mStatus;
    void setStatus(EventWindowStatus ews) { mStatus = ews; }
    friend class EWSet;
  };
}
#endif /* EVENTWINDOW_H */
