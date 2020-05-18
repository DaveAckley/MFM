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
#include "T2PacketBuffer.h"
#include "TimeoutAble.h"
#include "ITCIterator.h"
#include "RectIterator.h"

#define CIRCUIT_BITS 4   /* each for active and passive */
#define CIRCUIT_COUNT (1<<CIRCUIT_BITS)
#define ALL_CIRCUITS_BUSY 0xff

namespace MFM {

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD
  struct T2EventWindow; // FORWARD

  struct T2ITCPacketPoller : public TimeoutAble {
    // TimeoutAble API
    virtual void onTimeout(TimeQueue& srcTq) ;
    virtual const char * getName() const { return "PktPoll"; }

    T2ITCPacketPoller(T2Tile& tile) ;

    ~T2ITCPacketPoller() { }
    T2Tile & mTile;
    ITCIteration mIteration;
  };

  struct Circuit {
    CircuitNum mNumber; // 0..CIRCUIT_COUNT-1
    u8 mEW;     // 1..MAX_EWSLOTS
  };

  /**** ITC STATE MACHINE: EARLY STATES HACKERY ****/

#define ALL_ITC_STATES_MACRO()                                   \
  /*   name   vz,ch,mc,to,rc,sb,desc */                          \
  XX(INIT,     0, 0, 0, 1, 0, 0, "initialized state")            \
  XX(WAITCOMP, 1, 0, 0, 1, 0, 0, "wait for compatible physics")  \
  XX(LEAD0,    0, 0, 2, 1, 1, 0, "claim I am leader, nonce 0")   \
  XX(LEAD1,    0, 0, 2, 1, 1, 0, "claim I am leader, nonce 1")   \
  XX(WLEAD0,   0, 0, 2, 0, 0, 0, "await leader ack, nonce 0")    \
  XX(WLEAD1,   0, 0, 2, 0, 0, 0, "await leader ack, nonce 1")    \
  XX(FOLLOW,   0, 0, 2, 1, 1, 0, "declare I am follower")        \
  XX(WFOLLOW,  0, 0, 2, 1, 0, 0, "await follower ack")           \
  XX(CACHEXG,  0, 0, 2, 1, 1, 0, "synchronizing caches")         \
  XX(WCACHEXG, 0, 0, 2, 1, 0, 0, "wait for cache sync")          \
  XX(SYNCED,   1, 1, 2, 0, 0, 1, "intertile events active")      \

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
  
  typedef enum waitcode {
    WC_NOW,                     
    WC_HALF,   
    WC_FULL,   
    WC_LONG,   
    WC_RANDOM, 
    WC_RANDOM_SHORT, 
    MAX_WAIT_CODE
  } WaitCode;

  typedef enum waitms {
    WC_HALF_MS = 150,
    WC_FULL_MS = 300,

    WC_LONG_MIN_MS = 10000,
    WC_LONG_MAX_MS = 15000,
    WC_LONG_WIDTH = WC_LONG_MAX_MS - WC_LONG_MIN_MS+1,

    WC_RANDOM_MIN_MS = 30,
    WC_RANDOM_MAX_MS = 1500,

    WC_RANDOM_WIDTH = WC_RANDOM_MAX_MS - WC_RANDOM_MIN_MS+1,

    WC_RANDOM_SHORT_MIN_MS = 1,
    WC_RANDOM_SHORT_MAX_MS = 10,

    WC_RANDOM_SHORT_WIDTH = WC_RANDOM_SHORT_MAX_MS - WC_RANDOM_SHORT_MIN_MS+1

  } WaitMs;

  struct T2ITC : public TimeoutAble {
    virtual void onTimeout(TimeQueue& srcTq) ;
    virtual const char * getName() const ;

    T2ITC(T2Tile& tile, Dir6 dir6, const char * name) ;

    static bool isGingerDir6(Dir6 dir6) ;
    static bool isFredDir6(Dir6 dir6) { return !isGingerDir6(dir6); }
    bool isFred() const { return !isGinger(); }
    bool isGinger() const { return isGingerDir6(mDir6); }

    void scheduleWait(WaitCode wc) ;

    bool isVisibleUsable() ;  // true unless draining EWs before sync

    bool isCacheUsable() ;    // false unless ITC has reached cache sync

    static Rect getRectForTileInit(Dir6 dir6, u32 widthIn, u32 skipIn) ;

    Rect getRectForTileInit(u32 widthIn, u32 skipIn) { return getRectForTileInit(mDir6,widthIn,skipIn); }

    const Rect & getVisibleRect() const ;
    const Rect & getCacheRect() const ;
    const Rect & getVisibleAndCacheRect() const ;

    CircuitNum tryAllocateActiveCircuit() ;

    u32 activeCircuitsInUse() const ;

    void freeActiveCircuit(CircuitNum cn) ;

    void reset() ;             // Perform reset actions and enter ITCSN_INIT

    bool initialize() ;

    void resetAllCircuits() ;

    void pollPackets(bool dispatch) ;

    bool tryHandlePacket(bool dispatch) ;

    bool trySendPacket(T2PacketBuffer &pb) ;

    T2Tile & mTile;
    const Dir6 mDir6;
    const Dir8 mDir8;
    const char * mName;
    ITCStateNumber mStateNumber;
    T2ITCStateOps & getT2ITCStateOps() ;

    ITCStateNumber getITCSN() const { return mStateNumber; }
    void setITCSN(ITCStateNumber itcsn) ;

    Circuit mCircuits[2][CIRCUIT_COUNT]; // [0][] passive, [1][] active

    u8 mActiveFreeCount;
    CircuitNum mActiveFree[CIRCUIT_COUNT];

    int mFD;

    RectIterator mVisibleAtomsToSend;

    //// HELPERs
    u32 getCompatibilityStatus() ; // 0 closed 1 unknown compat 2 known compat
    s32 resolveLeader(ITCStateNumber theirimputedsn) ;
    void leadFollowOrReset(ITCStateNumber theirimputedsn) ;
    void startCacheSync(bool asLeader) ;
    bool sendCacheAtoms(T2PacketBuffer & pb) ;
    bool recvCacheAtoms(T2PacketBuffer & pb) ;
    bool tryReadAtom(ByteSource & in, UPoint & where, OurT2AtomBitVector & bv) ;

    const char * path() const;
    int open() ;
    int close() ;
    int getFD() const { return mFD; }
  };
}
#endif /* T2ITC_H */
