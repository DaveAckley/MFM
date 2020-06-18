#include "T2ITC.h"
#include "T2Tile.h"
#include "T2EventWindow.h"

#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>    // For close
#include <stdio.h>     // For snprintf
#include <errno.h>     // For errno

#include "dirdatamacro.h" 
#include "TraceTypes.h" 
#include "T2PacketBuffer.h" 

namespace MFM {

  T2ITCPacketPoller::T2ITCPacketPoller(T2Tile& tile)
    : mTile(tile)
    , mIteration(tile.getRandom(), 100)
  { }

  void T2ITCPacketPoller::onTimeout(TimeQueue& srcTq) {
    for (ITCIterator itr = mIteration.begin(); itr.hasNext(); ) {
      u32 dir6 = itr.next();
      T2ITC & itc = mTile.getITC(dir6);
      itc.pollPackets(true);
    }

    scheduleWait(WC_RANDOM_SHORT);
  }

  bool T2ITC::isGingerDir6(Dir6 dir6) {
    switch (dir6) {
#define XX(DC,fr,p1,p2,p3,p4) case DIR6_##DC: return !fr;
      DIRDATAMACRO()
#undef XX
    default:
        MFM_API_ASSERT_ARG(0);
    }
  }

  bool T2ITC::isVisibleUsable() const {
    const T2ITCStateOps & ops = getT2ITCStateOps();
    return ops.isVisibleUsable();
  }

  bool T2ITC::isCacheUsable() const {
    const T2ITCStateOps & ops = getT2ITCStateOps();
    return ops.isCacheUsable();
  }

  const SPoint T2ITC::getITCOrigin() const { return getVisibleRect().GetPosition(); }
  const SPoint T2ITC::getMateITCOrigin() const { return getCacheRect().GetPosition(); }
  const Rect & T2ITC::getVisibleRect() const { return mTile.getVisibleRect(mDir6); }
  const Rect & T2ITC::getCacheRect() const { return mTile.getCacheRect(mDir6); }
  const Rect & T2ITC::getVisibleAndCacheRect() const { return mTile.getVisibleAndCacheRect(mDir6); }

  Rect T2ITC::getRectForTileInit(Dir6 dir6, u32 widthIn, u32 skipIn) {
    switch (dir6) {
    case DIR6_ET:
      return Rect(T2TILE_WIDTH-(widthIn+skipIn),0,
                  widthIn,T2TILE_HEIGHT);
    case DIR6_SE:
      return Rect(T2TILE_WIDTH/2-CACHE_LINES,T2TILE_HEIGHT-(widthIn+skipIn),
                  T2TILE_WIDTH/2+CACHE_LINES, widthIn);
    case DIR6_SW:
      return Rect(0,T2TILE_HEIGHT-(widthIn+skipIn),
                  T2TILE_WIDTH/2+CACHE_LINES, widthIn);
    case DIR6_WT:
      return Rect(skipIn,0,
                  widthIn,T2TILE_HEIGHT);
    case DIR6_NW:
      return Rect(0,skipIn,
                  T2TILE_WIDTH/2+CACHE_LINES, widthIn);
    case DIR6_NE:
      return Rect(T2TILE_WIDTH/2-CACHE_LINES, skipIn,
                  T2TILE_WIDTH/2+CACHE_LINES, widthIn);
    }
    FAIL(ILLEGAL_STATE);
  }

  void T2ITC::onTimeout(TimeQueue& srcTq) {
    T2ITCStateOps & ops = getT2ITCStateOps();
    const u32 curenable = mTile.getKITCPoller().getKITCEnabledStatus(mDir8);
    const u32 minenable = ops.getMinimumEnabling();
    if (curenable < minenable) {
      LOG.Warning("%s enable=%d need=%d, resetting",
                  getName(), curenable, minenable);
      reset();
    } else {
      T2PacketBuffer pb;
      pb.Printf("%c%c",
                PKT_HDR_BITMASK_STANDARD_MFM | mDir8,
                xitcByte1(XITC_ITC_CMD,mStateNumber)
                );
      ops.timeout(*this, pb, srcTq);
    }
  }

  void T2ITC::pollPackets(bool dispatch) {
    if (mFD < 0) return; // Not open yet
    while (tryHandlePacket(dispatch)) { }
  }

  u32 T2ITC::getCompatibilityStatus() {
    return mTile.getKITCPoller().getKITCEnabledStatus(mDir8);
  }

  void T2ITC::handleCircuitPacket(T2PacketBuffer & pb) {
    u32 len = pb.GetLength();
    if (len < 2) {
      LOG.Warning("%s: %d byte circuit packet, ignored", getName(), len);
      return;
    }
    u8 xitc;
    if (!asXITC(pb,&xitc)) FAIL(ILLEGAL_STATE);
    switch (xitc) {
    default:
      FAIL(UNREACHABLE_CODE);   
      return;
    case XITC_CS_RING:          // Request lock (by caller)
      handleRingPacket(pb);
      break;
    case XITC_CS_ANSWER:        // Grant lock (by callee)
      handleAnswerPacket(pb);
      break;
    case XITC_CS_BUSY:          // Reject lock (by callee)
      FAIL(INCOMPLETE_CODE);
      break;
    case XITC_CS_TALK:          // Cache updates (by caller)
      handleCacheUpdatesPacket(pb);
      break;
    case XITC_CS_HANGUP:        // Release lock (by callee)
      handleHangUpPacket(pb);
      break;
    case XITC_CS_DROP:         // Discard lock (by caller)
      handleDropPacket(pb);
      break;
    }
  }

  void T2ITC::handleDropPacket(T2PacketBuffer & pb) {
    TLOG(DBG,"%s: Enter hDP", getName());
    // XXX THIS NEEDS REWRIT BECAUSE DROP IS RECVD BY PASSIVE SIDE
    FAIL(INCOMPLETE_CODE);
#if 0    
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    const char * pkt = pb.GetBuffer();
    u8 slotnum = pkt[1]&0xf;
    T2Tile & tile = T2Tile::get();
    T2EventWindow & ew = tile.getEWOrDie(slotnum);
    if (!ew.isActive()) {
      FAIL(INCOMPLETE_CODE); // VATDOOVEEDOO
    }
    MFM_API_ASSERT_NONNULL(ew);
    T2EventWindow & activeEW = *ew;

    unregisterEWRaw(activeEW); // Whether we had a circuit or not

    activeEW.dropActiveEW(true);
#endif
  }

  void T2ITC::handleRingPacket(T2PacketBuffer & pb) {
    u32 len = pb.GetLength();
    if (len < 5) {
      LOG.Warning("%s: Ring packet len==%d, ignored", getName(), len);
      return;
    }
    u8 cn;
    s8 sx, sy;
    bool ayoink;
    u8 radius;
    if (!asCSRing(pb, &cn, &sx, &sy, &ayoink, &radius))
      FAIL(UNREACHABLE_CODE);

    SPoint theirCtr(sx,sy);
    SPoint theirOrigin = getMateITCOrigin();
    SPoint ourCtr = theirCtr + theirOrigin;
    TLOG(DBG,"%s: HRP abs(%d,%d) themrel(%d,%d) yoink=%d",
              getName(),
              ourCtr.GetX(), ourCtr.GetY(),
              theirCtr.GetX(), theirCtr.GetY(),
              ayoink
              );
    if (radius > 4) {
      LOG.Warning("%s: Ring packet (%d,%d) radius==%d, ignored", getName(), sx, sy, radius);
      return;
    }

    T2PassiveEventWindow & pEW = *mPassiveEWs[cn]; // cn OK via asCSRing
    MFM_API_ASSERT_STATE(pEW.getEWSN() == EWSN_PINIT);

    pEW.initPassive(ourCtr, radius, ayoink);
    pEW.setEWSN(EWSN_PRESOLVE);

    if (!pEW.checkSiteAvailabilityForPassive()) { // false -> passive lost, nak sent
      FAIL(INCOMPLETE_CODE);
#if 0
      // Now what?  Just clean up passive?
      MFM_API_ASSERT_STATE(mCircuits[0][cn].mEW == passiveEW.slotNum());
      MFM_API_ASSERT_STATE(mCircuits[0][cn].mYoinkVal >= 0);
      mCircuits[0][cn].mEW = 0; // XXX really?  really really?  not max sumtin?
      mCircuits[0][cn].mYoinkVal = -1;
      passiveEW.finalizeEW();
      TLOG(DBG,"Passive cn %d released",cn);
#endif
      return;
    }

    if (!trySendAckPacket(cn)) {
      FAIL(INCOMPLETE_CODE);
    }

    Circuit & ci = pEW.getPassiveCircuit();
    ci.setCS(CS_ANSWERED); // We have answered the call
  }

  bool T2ITC::trySendAckPacket(CircuitNum cn) {
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              PKT_HDR_BITMASK_STANDARD_MFM | mDir8,
              xitcByte1(XITC_CS_ANSWER,cn)
              );
    return trySendPacket(pb);
  }

  void T2ITC::handleHangUpPacket(T2PacketBuffer & pb) {
    TLOG(DBG,"%s: Enter HUP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    u8 cn;
    if (!asCSHangup(pb, &cn))
      FAIL(ILLEGAL_ARGUMENT);

    Circuit * cp = mActiveEWCircuits[cn]; // cn okay by asCSHangup
    if (!cp) {
      TLOG(WRN,"%s HHU bad cn %d", getName(), cn);
      return; // XXX ? 
    }
    Circuit & ci = *cp;

    T2EventWindow & ew = ci.getEW();
    T2ActiveEventWindow * aewp = ew.asActiveEW();
    if (!aewp) {
      TLOG(WRN,"%s HHU cn %d not aew: %s", getName(), cn, ew.getName());
      return; // XXX ? 
    }
    T2ActiveEventWindow & aew = *aewp;
    
    ci.unbindCircuit(); // We are done with this job!
    if (aew.getEWSN() != EWSN_AWACKS) 
      FAIL(INCOMPLETE_CODE); // DITTOO
    else aew.handleHangUp(*this);
  }

  void T2ITC::handleCacheUpdatesPacket(T2PacketBuffer & pb) {
    TLOG(DBG,"%s: Enter HCUP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    u8 cn;
    if (!asCSTalk(pb,&cn))
      FAIL(ILLEGAL_ARGUMENT);
    T2PassiveEventWindow & passiveEW = *mPassiveEWs[cn];
    MFM_API_ASSERT_STATE(passiveEW.getEWSN() == EWSN_PWCACHE);
    Circuit & ci = passiveEW.getPassiveCircuit();
    MFM_API_ASSERT_STATE(ci.getCS() == CS_ANSWERED);
    ci.setCS(CS_TALKED);
    passiveEW.applyCacheUpdatesPacket(pb, *this);
  }

  void T2ITC::hangUpPassiveEW(T2EventWindow & passiveEW, CircuitNum cn) {
    MFM_API_ASSERT_ARG(passiveEW.getEWSN() == EWSN_PWCACHE);
    MFM_API_ASSERT_ARG(cn < MAX_EWSLOT);
    // SEND HANG UP
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              PKT_HDR_BITMASK_STANDARD_MFM | mDir8,
              xitcByte1(XITC_CS_HANGUP,cn)
              );
    if (!trySendPacket(pb)) {
      FAIL(INCOMPLETE_CODE); // DO WE HAVE TO BLOCK OR BLOW UP SOMETHING NOW?
    }
  }
  
  void T2ITC::handleAnswerPacket(T2PacketBuffer & pb) {
    TLOG(DBG,"%s: Enter HAP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    u8 cn;
    if (!asCSAnswer(pb,&cn))
      FAIL(ILLEGAL_ARGUMENT);

    Circuit * cp = mActiveEWCircuits[cn]; // cn okay by asCSAnswer
    if (!cp) {
      TLOG(WRN,"%s HAP bad cn %d", getName(), cn);
      return; // XXX ? 
    }

    Circuit & ci = *cp;
    if (ci.getCS() != CS_RUNG) {
      TLOG(WRN,"%s HAP bad ci %s for cn %d",
           getName(),
           ci.getName(),
           cn);
      return; // XXX ? 
    }

    ci.setCS(CS_ANSWERED); // We have this lock

    T2EventWindow & ew = ci.getEW();
    T2ActiveEventWindow * aewp = ew.asActiveEW();
    if (!aewp) {
      TLOG(WRN,"%s HAP cn %d not aew: %s", getName(), cn, ew.getName());
      return; // XXX ? 
    }
    T2ActiveEventWindow & aew = *aewp;

    TLOG(DBG,"%s %s HAP", getName(), aew.getName());
    if (aew.getEWSN() != EWSN_AWLOCKS) {
      TLOG(WRN,"%s HAP bad aew state %s for cn %d",
           getName(), aew.getName(), cn);
      aew.abortEW(); // XXX ??
      return;
    }
    else aew.handleACK(*this);
  }

  bool T2ITC::tryHandlePacket(bool dispatch) {
    T2PacketBuffer pb;
    const u32 MAX_PACKET_SIZE = 255;
    int len = pb.Read(mFD, MAX_PACKET_SIZE);
    const char * packet = pb.GetBuffer();
    if (len < 0) {
      if (errno == EAGAIN) return false;
      if (errno == ERESTART) return true; // try me again
      LOG.Error("%s: Read failed on mFD %d: %s",
                getName(), mFD, strerror(errno));
      return false;
    }
    if (!dispatch) return true;
    if (len == 1)
      LOG.Debug("%s: Recv %d/0x%02x", getName(), len, packet[0]);
    else
      LOG.Debug("%s: Recv %d/0x%02x 0x%02x%s", getName(), len,
                packet[0], packet[1], len > 2? " ..." : "");
    if (true/*TRACEACTIVE*/) {
      Trace evt(*this, TTC_ITC_PacketIn);
      evt.payloadWrite().WriteBytes((const u8*) packet, len);
      mTile.trace(evt);
    }
    if (len < 2) LOG.Warning("%s one byte 0x%02x packet, ignored", getName(), packet[0]);
    else {
      u8 xitc;
      if (!asXITC(pb, &xitc)) FAIL(ILLEGAL_STATE);
      u8 sn;
      if (asITC(pb,&sn)) { // ITC state command
        ITCStateNumber psn = (ITCStateNumber) sn;
        T2ITCStateOps * ops = T2ITCStateOps::getOpsOrNull(psn);
        MFM_API_ASSERT_NONNULL(ops);
        ops->receive(*this, pb, mTile.getTQ());
      } else if (xitc >= XITC_CS_MIN_VAL && xitc <= XITC_CS_MAX_VAL) {
        handleCircuitPacket(pb);
      } else {
        LOG.Warning("%s xitc type %d packet, ignored", getName(), xitc);
      }
    }
    return true;
  }

  bool T2ITC::trySendPacket(T2PacketBuffer & pb) {
    s32 packetlen = pb.GetLength();
    const char * bytes = pb.GetBuffer();
    s32 len = ::write(mFD, bytes, packetlen);
    if (packetlen == 1)
      LOG.Debug("  %s wrote %d/0x%02x == %d",
                getName(),
                packetlen,
                bytes[0],
                len);
    else
      LOG.Debug("  %s wrote %d/0x%02x 0x%02x%s== %d",
                getName(),
                packetlen,
                bytes[0],bytes[1],
                packetlen > 2 ? " ..." : "",
                len);
    if (len < 0) {
      if (errno == EAGAIN) return false; // No room, try again
      if (errno == ERESTART) return false; // Interrupted, try again
    }
    if (len != packetlen) return false;  // itcpkt LKM doesn't actually support partial write
    if (true /*XXX TRACE ACTIVE*/) {
      Trace evt(*this, TTC_ITC_PacketOut);
      evt.payloadWrite().WriteBytes((const u8*) bytes,len);
      mTile.trace(evt);
    }
    ++mPacketsShipped;                   // another day, 
    return true;                         // 'the bird is away'
  }

  void T2ITC::setITCSN(ITCStateNumber itcsn) {
    MFM_API_ASSERT_ARG(itcsn >= 0 && itcsn < MAX_ITC_STATE_NUMBER);
    mTile.trace(*this, TTC_ITC_StateChange,"%c",(u8) itcsn);
    mStateNumber = itcsn;
  }

  void T2ITC::reset() {
    if (mFD >= 0) close();
    initializeFD();
    abortAllActiveCircuits();
    mCacheReceiveComplete = false;
    setITCSN(ITCSN_SHUT);
    scheduleWait(WC_RANDOM_SHORT); // Delay in case reset-looping
  }

  bool T2ITC::initializeFD() {
    int ret = open();
    if (ret < 0) {
      LOG.Error("%s open failed: %s", getName(), strerror(-ret));
      return false;
    }
    pollPackets(false); // First flush packets

    return true;
    /*
    T2PacketBuffer pb;    // Then try to ping for status
    pb.WriteByte(0xc0|LCLSTD_MFM_TO_LKM_PING); 

    return trySendPacket(pb);
    */
  }

  void T2ITC::abortAllActiveCircuits() {
    for (u32 i = 0; i < MAX_EWSLOT; ++i) {
      if (mActiveEWCircuits[i] != 0)
        mActiveEWCircuits[i]->abortCircuit();
      MFM_API_ASSERT_NULL(mActiveEWCircuits[i]);
    }
  }

  T2ITC::T2ITC(T2Tile& tile, Dir6 dir6, const char * name)
    : mTile(tile)
    , mDir6(dir6)
    , mDir8(mapDir6ToDir8(dir6))
    , mName(name)
    , mPacketsShipped(0u)
    , mStateNumber(ITCSN_SHUT)
    , mFD(-1)
    , mActiveEWCircuits{ 0 }
    , mActiveEWCircuitCount(0)
    , mPassiveEWs{ 0 }
    , mVisibleAtomsToSend()
    , mCacheReceiveComplete(false)
    {
      for (u32 i = 0; i < MAX_EWSLOT; ++i) 
        mPassiveEWs[i] = new T2PassiveEventWindow(mTile, i, mName, *this);
    }

  T2ITC::~T2ITC() {
    for (u32 i = 0; i < MAX_EWSLOT; ++i) {
      delete mPassiveEWs[i];
      mPassiveEWs[i] = 0;
    }
  }

  /**** LATE ITC STATES HACKERY ****/

  /*** DEFINE STATEOPS SINGLETONS **/
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) static T2ITCStateOps_##NAME singletonT2ITCStateOps_##NAME;
  ALL_ITC_STATES_MACRO()
#undef XX

  /*** DEFINE ITCSTATENUMBER -> STATEOPS MAPPING **/
  T2ITCStateOps::T2ITCStateArray T2ITCStateOps::mStateOpsArray = {
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) &singletonT2ITCStateOps_##NAME,
  ALL_ITC_STATES_MACRO()
#undef XX
    0
  };

  T2ITCStateOps * T2ITCStateOps::getOpsOrNull(ITCStateNumber sn) {
    if (sn >= MAX_ITC_STATE_NUMBER) return 0;
    return mStateOpsArray[sn];
  }

  /*** DEFINE STUBBED-OUT STATE METHODS **/
#define YY00(NAME,FUNC) 
#define YY01(NAME,FUNC) 
#define YY10(NAME,FUNC) 
#define YY11(NAME,FUNC)                                                 \
void T2ITCStateOps_##NAME::FUNC(T2ITC & ew, T2PacketBuffer & pb, TimeQueue& tq) { \
  LOG.Error("%s is a stub", __PRETTY_FUNCTION__);                                 \
  DIE_UNIMPLEMENTED();                                                  \
}                                                                       \
 
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) \
  YY##CUSTO##STUB(NAME,timeout)                        \
  YY##CUSRC##STUB(NAME,receive)                        \
  
  ALL_ITC_STATES_MACRO()
#undef XX
#undef YY11
#undef YY10
#undef YY01
#undef YY00

  /*** STATE NAMES AS STRING **/
  const char * itcStateName[] = {
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) #NAME,
  ALL_ITC_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getITCStateName(ITCStateNumber sn) {
    if (sn >= MAX_ITC_STATE_NUMBER) return "illegal";
    return itcStateName[sn];
  }

  /*** STATE DESCRIPTIONS AS STRING **/
  const char * itcStateDesc[] = {
#define XX(NAME,VIZ,CCH,MINCOMP,CUSTO,CUSRC,STUB,DESC) DESC,
  ALL_ITC_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getITCStateDescription(ITCStateNumber sn) {
    if (sn >= MAX_ITC_STATE_NUMBER) return "illegal";
    return itcStateDesc[sn];
  }

  const char * T2ITC::getName() const {
    static char buf[100];
    snprintf(buf,100,"ITC/%s:%s",
             mName,
             getITCStateName(mStateNumber));
    return buf;
  }

  //// DEFAULT HANDLERS FOR T2ITCStateOps
  void T2ITCStateOps::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Warning("%s default mode timeout, resetting",itc.getName());
    itc.reset();
  }

  void T2ITCStateOps::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Warning("%s default mode receive packet len %d , resetting",
                itc.getName(), pb.GetLength());
    itc.reset();
  }

  const T2ITCStateOps & T2ITC::getT2ITCStateOps() const {
    if (mStateNumber >= T2ITCStateOps::mStateOpsArray.size())
      FAIL(ILLEGAL_STATE);
    const T2ITCStateOps * ops = T2ITCStateOps::mStateOpsArray[mStateNumber];
    if (!ops)
      FAIL(ILLEGAL_STATE);
    return *ops;
  }

  s8 T2ITC::getYoinkVal(CircuitNum cn, bool forActive) const {
    FAIL(INCOMPLETE_CODE);
    return -1;
    //    MFM_API_ASSERT_ARG(cn < MAX_EWSLOT);
    //    return mCircuits[forActive ? 1 : 0][cn].mYoinkVal;
  }

  void T2ITC::registerActiveCircuitRaw(Circuit & ct) {
    const T2EventWindow & ew = ct.getEW();
    MFM_API_ASSERT_STATE(ew.isActiveEW());
    EWSlotNum sn = ew.getSlotNum();
    MFM_API_ASSERT_STATE(sn < MAX_EWSLOT);
    MFM_API_ASSERT_NULL(mActiveEWCircuits[sn]);
    MFM_API_ASSERT_STATE(isVisibleUsable());
    mActiveEWCircuits[sn] = &ct;
    ++mActiveEWCircuitCount;
  }

  void T2ITC::unregisterActiveCircuitRaw(Circuit & ct) {
    const T2EventWindow & ew = ct.getEW();
    MFM_API_ASSERT_STATE(ew.isActiveEW());
    EWSlotNum sn = ew.getSlotNum();
    MFM_API_ASSERT_STATE(sn < MAX_EWSLOT);
    if (mActiveEWCircuits[sn] != 0) {
      MFM_API_ASSERT_STATE(mActiveEWCircuitCount > 0);
      mActiveEWCircuits[sn] = 0;
      --mActiveEWCircuitCount;
    }
    if (mActiveEWCircuitCount == 0 && getITCSN() == ITCSN_DRAIN)
      scheduleWait(WC_NOW); // bump
  }

  const char * T2ITC::path() const {
    static char buf[100];
    snprintf(buf,100,"/dev/itc/mfm/%s",mName);
    return buf;
  }

  int T2ITC::open() {
    int ret = ::open(path(),O_RDWR|O_NONBLOCK);
    if (ret < 0) return -errno;
    mFD = ret;
    return ret;
  }

  int T2ITC::close() {
    int ret = ::close(mFD);
    mFD = -1;
    if (ret < 0) return -errno;
    return ret;
  }

  void T2ITC::startCacheSync() {
    mVisibleAtomsToSend.begin(getVisibleRect());
    mCacheAtomsSent = 0;
    mCacheAtomsReceived = 0;
    setITCSN(ITCSN_CACHEXG);
    scheduleWait(WC_NOW);
  }

  bool T2ITC::sendVisibleAtoms(T2PacketBuffer & pb) { // Which presto-change-o arrive via recvCacheAtoms..
    const u32 BYTES_PER_ATOM = OurT2Atom::BPA/8; // Bits Per Atom/Bits Per Byte
    const u32 BYTES_PER_DIM = 1; // Sending u8 dimensions inside visibleRect
    const u32 BYTES_PER_COORD = 2*BYTES_PER_DIM;
    const u32 BYTES_PER_SITE = BYTES_PER_COORD+BYTES_PER_ATOM;
    Sites & sites = mTile.getSites();
    UPoint pos = MakeUnsigned(mVisibleAtomsToSend.getRect().GetPosition());
    RectIterator hold = mVisibleAtomsToSend;
    bool alreadyDone = !mVisibleAtomsToSend.hasNext();
    u32 pending = 0;
    while (mVisibleAtomsToSend.hasNext()) {
      if (pb.CanWrite() < (s32) BYTES_PER_SITE) break;
      SPoint coord = mVisibleAtomsToSend.next();
      UPoint ucoord = MakeUnsigned(coord);
      UPoint coordInVis = ucoord - pos;
      pb.Printf("%c%c",(u8) coordInVis.GetX(), (u8) coordInVis.GetY());
      
      OurT2Site & site = sites.get(ucoord);
      OurT2Atom & atom = site.GetAtom();
      const OurT2AtomBitVector & bv = atom.GetBits();
      bv.PrintBytes(pb);
      ++pending;
    }
    if (trySendPacket(pb)) {
      mCacheAtomsSent += pending;
      return alreadyDone; // true when we just sent an empty CACHEXG
    }
    // Grrr
    mVisibleAtomsToSend = hold; // All that work.
    return false;
  }

  bool T2ITC::tryReadAtom(ByteSource & in, UPoint & where, OurT2AtomBitVector & bv) {
    u8 x,y;
    if (in.Scanf("%c%c",&x,&y) != 2) return false;
    OurT2AtomBitVector tmpbv;
    if (!tmpbv.ReadBytes(in)) return false;
    where.SetX(x);
    where.SetY(y);
    bv = tmpbv;
    return true;
  }

  bool T2ITC::recvCacheAtoms(T2PacketBuffer & pb) {
    CharBufferByteSource cbs = pb.AsByteSource();
    u8 byte0=0, byte1=0;
    if ((cbs.Scanf("%c%c",&byte0,&byte1) != 2) ||
        ((byte0 & 0xf0) != PKT_HDR_BITMASK_STANDARD_MFM) ||
        (byte1 != xitcByte1(XITC_ITC_CMD,ITCSN_CACHEXG))) {
      LOG.Error("%s: Bad CACHEXG hdr 0x%02x 0x%02x",
                getName(), byte0, byte1);
      reset();
      return true;
    }

    Sites & sites = mTile.getSites();
    const Rect cache = getCacheRect();
    const SPoint pos = cache.GetPosition();
    u32 count = 0;
    UPoint offset;
    OurT2Atom tmpatom;
    OurT2AtomBitVector & tmpbv = tmpatom.GetBits();

    while (tryReadAtom(cbs, offset, tmpbv)) {
      SPoint dest = pos+MakeSigned(offset);
      if (!cache.Contains(dest)) {
        LOG.Error("%s: (%d,%d) ATTEMPT TO ESCAPE CACHE",
                  getName(), dest.GetX(), dest.GetY());
        reset();
        return true;
      }
      if (!tmpatom.IsSane()) {
        LOG.Error("%s: RECEIVED INSANE ATOM",getName());
        reset();
        return true;
      }
      OurT2Site & site = sites.get(MakeUnsigned(dest));
      OurT2Atom & atom = site.GetAtom();
      atom = tmpatom;  // BAM
      ++mCacheAtomsReceived;
      ++count;
    }
    return count == 0; // CACHEXG w/no atoms means end
  }

  /////////////////// CUSTOM T2ITCStateOps HANDLERS

  void T2ITCStateOps_SHUT::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getCompatibilityStatus() >= 2) {
      itc.mTile.addRandomSyncTag(pb);
      itc.trySendPacket(pb);
    }
    itc.scheduleWait(WC_MEDIUM);
  }

  void T2ITCStateOps_SHUT::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() > ITCSN_SHUT) itc.reset();
    itc.setITCSN(ITCSN_DRAIN);
    itc.scheduleWait(WC_RANDOM_SHORT);
  }

  void T2ITCStateOps_DRAIN::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.activeCircuitsInUse() > 0) itc.reset();
    else itc.startCacheSync();
  }

  void T2ITCStateOps_CACHEXG::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    bool sendStarted = itc.isCacheSendStarted(); // Check before trying to ship..
    bool sendDone = itc.sendVisibleAtoms(pb);  // True when all atoms shipped
    bool recvStarted = itc.isCacheReceiveStarted(); 
    bool recvDone = itc.isCacheReceiveComplete(); 
    if (sendDone && recvDone) itc.setITCSN(ITCSN_OPEN);
    if (sendStarted && sendDone && !recvStarted) // We Guess Their Engine Is Dead
      itc.reset();
    else
      itc.scheduleWait(WC_RANDOM_SHORT); // For either state
  }

  void T2ITCStateOps_CACHEXG::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() < ITCSN_DRAIN) itc.reset();
    if (itc.recvCacheAtoms(pb)) itc.setCacheReceiveComplete();
    itc.scheduleWait(WC_NOW); // Got something; take a look
  }

  void T2ITCStateOps_OPEN::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    itc.mTile.addRandomSyncTag(pb);
    itc.trySendPacket(pb); // Hi neighbor
    itc.scheduleWait(WC_LONG);
  }

  void T2ITCStateOps_OPEN::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() < ITCSN_CACHEXG) itc.reset();
  }
}

