/* -*- C++ -*- */
#ifndef T2FLASHTRAFFICMANAGER_H
#define T2FLASHTRAFFICMANAGER_H

#include "TimeoutAble.h"
#include "ITCIterator.h"
#include "T2PacketBuffer.h"
#include "Panel.h"
#include "dirdatamacro.h"  // For DIR6*

namespace MFM {

  struct T2FlashCommandLabel; // FORWARD
  
#define ALL_T2FLASH_BUTTON_NAMES()      \
  XX(boot,1)                            \
  XX(off,1)                             \
  XX(tile,1)                            \
  XX(sites,1)                           \
  XX(less,0)                            \
  XX(down,0)                            \
  XX(dist,0)                            \
  XX(up,0)                              \
  XX(more,0)                            \
  XX(dismiss,0)                         \
  XX(clear,0)                           \
  XX(gogogo,0)                          \

  
  enum T2FlashCmd {
#define XX(N,OP) T2FLASH_CMD_##N,
     ALL_T2FLASH_BUTTON_NAMES()
     T2FLASH_CMD__COUNT
  };
#undef XX      

  struct T2FlashTrafficManager : public TimeoutAble {
    typedef TimeoutAble Super;

    virtual const char * getName() const { return "FlashMgr"; }

    T2FlashCmd findCmd(const char * name) const ;

    const char * getPanelName(T2FlashCmd cmd) const ;

    bool isOp(T2FlashCmd cmd) const ;

    void doCmd(T2FlashCmd cmd) ;

    void onClick(T2FlashCommandLabel * button) ;

    T2FlashTrafficManager() ;

    void init() ;

    virtual void onTimeout(TimeQueue& srcTQ) ;

    virtual ~T2FlashTrafficManager() ;

    s32 setRange(s32 range) ;
    s32 getRange() const ;

    void clearPending() ;
    void setPending(T2FlashCmd cmd) ;
    void executePending() ;
    T2FlashCmd getPending() const ;
    void showPending() const ;

    void dismissPanel() ;

    T2FlashCommandLabel & getGOGOGO() const ;

  private:

    void showRange() const ;

    void processTraffic() ;

    void handleInboundTraffic() ;

    void handleFlashPacket(T2PacketBuffer & pb) ;

    void sendFlashPacket(T2FlashCmd cmd, u32 ttl) ;

    u32 mTTL;
    
    s32 mLastIndex;

    T2FlashCmd mPendingCmd;

    int open() ;
    int close() ;
    const char * path() const ;
    bool flushPendingPackets() ;

    int mFD;
  };
}

#endif /* T2FLASHTRAFFICMANAGER_H */
