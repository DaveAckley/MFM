/* -*- C++ -*- */
#ifndef T2FLASHTRAFFICMANAGER_H
#define T2FLASHTRAFFICMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <set>

#include "T2Types.h"
#include "TimeoutAble.h"  // for time_before
#include "ITCIterator.h"
#include "T2PacketBuffer.h"
#include "FlashTraffic.h"
#include "Panel.h"
#include "dirdatamacro.h"  // For DIR6*
#include "T2Utils.h"

#include "FlashCommands.h"

namespace MFM {

  struct T2FlashCommandLabel; // FORWARD

  typedef std::map<std::string,u32> PanelSuffixToFlashCmd;
  typedef std::map<u32,std::string> U32ToString;

  struct TimedFlashTraffic {
    TimedFlashTraffic(const FlashTraffic packet) ;
    const FlashTraffic mPacket;
    const u32 mWhen;
    bool operator<(const TimedFlashTraffic & rhs) const {
      return time_before(mWhen,rhs.mWhen);
    }
  };
  typedef std::multiset<TimedFlashTraffic> MultisetTimedFlashTraffic;
  typedef std::map<u16,FlashTraffic> OriginKeyToFlashTraffic;

  struct T2FlashTrafficManager : public TimeoutAble {
    typedef TimeoutAble Super;

    virtual const char * getName() const { return "FlashMgr"; }

    T2FlashControlCmd findControlCmd(const char * name) const;

    T2FlashCmd findCmd(const char * name) const ;
    const char * getCmdName(T2FlashCmd cmd) const ;

    const char * getPanelName(T2FlashCmd cmd) const ;

    bool isOp(T2FlashCmd cmd) const ;

    bool matchesPacketInPQ(const FlashTraffic & packet, bool & diffOrigin) const ;

    bool matchesPreviousFromOrigin(const FlashTraffic & packet) const ;

    void acceptPacket(const FlashTraffic & packet) ;


    //    void doCmd(T2FlashCmd cmd) ;
    void doControlCmd(T2FlashControlCmd cmd) ;

    void onClick(T2FlashCommandLabel * button) ;

    T2FlashTrafficManager() ;

    void init() ;

    virtual void onTimeout(TimeQueue& srcTQ) ;

    virtual ~T2FlashTrafficManager() ;

    s32 setRange(s32 range) ;
    s32 getRange() const ;

    void clearPrepared() ;
    void setPrepared(T2FlashCmd cmd) ;
    void launchPreparedCommand() ;
    T2FlashCmd getPrepared() const ;
    T2FlashCommandLabel & getPreparedPanel() const ;
    void showPrepared() const ;

    void dismissPanel() ;

    T2FlashCommandLabel & getGOGOGO() const ;

    void report(const FlashTraffic pkt, ByteSink& to) ;

    void shipAndExecuteFlashDump() ;

  private:
#if 0
    PanelSuffixToFlashCmd m_suffixToCmd;
#endif
    U32ToString m_cmdToCmdName;
    U32ToString m_cmdToButtonPanelName;

    void initMaps() ;
    void showRange() const ;

    void processTraffic() ;

    void handleInboundTraffic() ;

    void handleInboundPacket(T2PacketBuffer & pb) ;

    void receiveFlashPacket(const FlashTraffic & packet) ;

    void sendFlashPacket(const FlashTraffic & packet) ;

    void setGoGoColors(u32 now) ;

    void managePending(TimeQueue& srcTQ) ;

    void executeFlashTrafficCommand(T2FlashCmd cmd) ;

    u32 mTTL;
    
    s32 mLastIndex;

    FlashTraffic mPreparedCmd;
    OriginKeyToFlashTraffic mOriginKeyToFlashTraffic;
    MultisetTimedFlashTraffic mMultisetTimedFlashTraffic;

    int open() ;
    int close() ;
    const char * path() const ;
    bool flushPendingPackets() ;

    int mFD;
  };
}

#endif /* T2FLASHTRAFFICMANAGER_H */
