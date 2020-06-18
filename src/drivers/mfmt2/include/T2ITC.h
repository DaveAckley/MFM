/* -*- C++ -*- */
#ifndef T2ITC_H
#define T2ITC_H

#include <vector>

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Rect.h"

// LKM files
#include "itcpktevent.h"

// Spike files
#include "T2Types.h"
#include "T2EventWindow.h"
#include "T2PacketBuffer.h"
#include "TimeoutAble.h"
#include "ITCIterator.h"
#include "RectIterator.h"

#define ALL_CIRCUITS_BUSY 0xff

namespace MFM {

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD

  struct T2ITCPacketPoller : public TimeoutAble {
    // TimeoutAble API
    virtual void onTimeout(TimeQueue& srcTq) ;
    virtual const char * getName() const { return "PktPoll"; }

    T2ITCPacketPoller(T2Tile& tile) ;

    ~T2ITCPacketPoller() { }
    T2Tile & mTile;
    ITCIteration mIteration;
  };

  /**** ITC STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_ITC_STATES_MACRO()                                   \
  /*   name   vz,ch,mc,to,rc,sb,desc */                          \
  XX(SHUT,     1, 0, 0, 1, 1, 0, "EWs running locally")          \
  XX(DRAIN,    0, 0, 0, 1, 0, 0, "drain EWs to settle cache")    \
  XX(CACHEXG,  0, 0, 2, 1, 1, 0, "exchange cache atoms")         \
  XX(OPEN,     1, 1, 2, 1, 1, 0, "intertile EWs active")         \

  /*** DECLARE STATE NUMBERS **/
  typedef enum itcstatenumber {

#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) ITCSN_##NAME,
  ALL_ITC_STATES_MACRO()
#undef XX

    MAX_ITC_STATE_NUMBER
  } ITCStateNumber;

  ////BASE CLASS OF ALL T2ITC STATE MACHINE HANDLER CLASSES
  struct T2ITCStateOps {
    typedef std::vector<T2ITCStateOps *> T2ITCStateArray;
    static T2ITCStateArray mStateOpsArray;
    static T2ITCStateOps * getOpsOrNull(ITCStateNumber sn) ;

    virtual bool isVisibleUsable() const = 0;
    virtual bool isCacheUsable() const = 0;
    virtual u32 getMinimumEnabling() const = 0;
    virtual void timeout(T2ITC & itc, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual void receive(T2ITC & itc, T2PacketBuffer &pb, TimeQueue& tq) ;
    virtual const char * getStateName() const = 0;
    virtual const char * getStateDescription() const = 0;
    virtual ~T2ITCStateOps() { }
  };

  /*** DECLARE PER-STATE SUBCLASSES ***/
#define YY0(FUNC) 
#define YY1(FUNC) virtual void FUNC(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) ;
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC)                \
  struct T2ITCStateOps_##NAME : public T2ITCStateOps {                \
    YY##CUSTO(timeout)                                                \
    YY##CUSRC(receive)                                                \
    virtual bool isVisibleUsable() const { return VIZ!=0; }           \
    virtual bool isCacheUsable() const { return CCH!=0; }             \
    virtual u32 getMinimumEnabling() const { return MINCOMP; }        \
    virtual const char * getStateName() const { return #NAME; }       \
    virtual const char * getStateDescription() const { return DESC; } \
    virtual ~T2ITCStateOps_##NAME() { }                               \
  };                                                                  \

  ALL_ITC_STATES_MACRO()
#undef XX
#undef YY1
#undef YY0
  
  inline u8 xitcByte1(XITCCode xitc, u8 arg) {
    return
      (u8) (((xitc<<PKT_HDR_BYTE1_XITC_POS) & PKT_HDR_BYTE1_BITMASK_XITC) |
            (arg & PKT_HDR_BYTE1_BITMASK_XITC_SN));
  }

  struct T2ITC : public TimeoutAble {
    virtual void onTimeout(TimeQueue& srcTq) ;
    virtual const char * getName() const ;

    T2ITC(T2Tile& tile, Dir6 dir6, const char * name) ;

    virtual ~T2ITC() ;

    static bool isGingerDir6(Dir6 dir6) ;
    static bool isFredDir6(Dir6 dir6) { return !isGingerDir6(dir6); }
    bool isFred() const { return !isGinger(); }
    bool isGinger() const { return isGingerDir6(mDir6); }

    bool isVisibleUsable() const;  // true unless draining EWs before sync

    bool isCacheUsable() const;    // false unless ITC has reached cache sync

    static Rect getRectForTileInit(Dir6 dir6, u32 widthIn, u32 skipIn) ;

    Rect getRectForTileInit(u32 widthIn, u32 skipIn) { return getRectForTileInit(mDir6,widthIn,skipIn); }

    const SPoint getITCOrigin() const ; // EW centers xmit relative to this
    const SPoint getMateITCOrigin() const ; 
    const Rect & getVisibleRect() const ;
    const Rect & getCacheRect() const ;
    const Rect & getVisibleAndCacheRect() const ;

    u32 activeCircuitsInUse() const { return mActiveEWCircuitCount; }

    s8 getYoinkVal(CircuitNum cn, bool forActive) const ;

    void reset() ;             // Perform reset actions and enter ITCSN_INIT

    bool initializeFD() ;

    void abortAllActiveCircuits() ;

    void pollPackets(bool dispatch) ;

    bool tryHandlePacket(bool dispatch) ;

    void handleCircuitPacket(T2PacketBuffer &pb) ;

    void handleRingPacket(T2PacketBuffer & pb) ;

    void handleAnswerPacket(T2PacketBuffer & pb) ;

    void handleCacheUpdatesPacket(T2PacketBuffer & pb) ;

    void handleHangUpPacket(T2PacketBuffer & pb) ;

    void handleDropPacket(T2PacketBuffer & pb) ;

    bool trySendAckPacket(CircuitNum cn) ;

    bool trySendPacket(T2PacketBuffer &pb) ;

    void hangUpPassiveEW(T2EventWindow & ew, CircuitNum cn) ;


    T2Tile & mTile;
    const Dir6 mDir6;
    const Dir8 mDir8;
    const char * mName;

    ITCStateNumber getITCSN() const { return mStateNumber; }
    void setITCSN(ITCStateNumber itcsn) ;

    void registerActiveCircuitRaw(Circuit & ct) ;
    void unregisterActiveCircuitRaw(Circuit & ct) ;
    
    bool isCacheSendStarted() const { return mCacheAtomsSent > 0; }
    bool isCacheReceiveStarted() const { return mCacheAtomsReceived > 0; }
    bool isCacheReceiveComplete() const { return mCacheReceiveComplete; }
    void setCacheReceiveComplete() { mCacheReceiveComplete = true; }

    //// HELPERs
    u32 getPacketsShipped() const { return mPacketsShipped; }
    u32 getCompatibilityStatus() ; // 0 closed 1 unknown compat 2 known compat
    s32 resolveLeader(ITCStateNumber theirimputedsn) ;
    void leadFollowOrReset(ITCStateNumber theirimputedsn) ;
    void startCacheSync() ;
    bool sendVisibleAtoms(T2PacketBuffer & pb) ;
    bool recvCacheAtoms(T2PacketBuffer & pb) ;
    bool tryReadAtom(ByteSource & in, UPoint & where, OurT2AtomBitVector & bv) ;

    const char * path() const;
    int open() ;
    int close() ;
    int getFD() const { return mFD; }

  private:
    u32 mPacketsShipped;
    ITCStateNumber mStateNumber;
    T2ITCStateOps & getT2ITCStateOps() {
      return
        const_cast <T2ITCStateOps &>
        (static_cast<const T2ITC*>(this)->getT2ITCStateOps());
    }
    const T2ITCStateOps & getT2ITCStateOps() const;

    int mFD;

    Circuit *(mActiveEWCircuits[MAX_EWSLOT]); // Links to in-use active EWs
    u32 mActiveEWCircuitCount;                // # of non-zero

    T2PassiveEventWindow * mPassiveEWs[MAX_EWSLOT]; // All our passive EWs

    u32 mCacheAtomsSent;
    RectIterator mVisibleAtomsToSend;
    u32 mCacheAtomsReceived;
    bool mCacheReceiveComplete;

  };

  const char * getITCStateName(ITCStateNumber sn) ;

}
#endif /* T2ITC_H */
