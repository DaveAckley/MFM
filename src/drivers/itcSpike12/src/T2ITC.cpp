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

  bool T2ITC::isVisibleUsable() {
    T2ITCStateOps & ops = getT2ITCStateOps();
    return ops.isVisibleUsable();
  }

  bool T2ITC::isCacheUsable() {
    T2ITCStateOps & ops = getT2ITCStateOps();
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
                0xa0|mDir8,   /*standard+urgent to dir8*/
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
    //LOG.Debug("%s: Enter HCP", getName());
    u32 len = pb.GetLength();
    if (len < 2) {
      LOG.Warning("%s: %d byte circuit packet, ignored", getName(), len);
      return;
    }
    const char * pkt = pb.GetBuffer();
    u8 xitc = (pkt[1]>>4)&0x7;
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
    case XITC_CS_FLASH:         // Discard lock (by caller)
      FAIL(INCOMPLETE_CODE);
      break;
    }
  }

  void T2ITC::handleRingPacket(T2PacketBuffer & pb) {
    LOG.Debug("%s: Enter HRP", getName());
    u32 len = pb.GetLength();
    if (len < 5) {
      LOG.Warning("%s: Ring packet len==%d, ignored", getName(), len);
      return;
    }
    const char * pkt = pb.GetBuffer();
    u8 cn = pkt[1]&0xf;
    s8 sx = (s8) pkt[2];
    s8 sy = (s8) pkt[3];
    SPoint theirCtr(sx,sy);
    SPoint theirOrigin = getMateITCOrigin();
    SPoint ourCtr = theirCtr + theirOrigin;
    bool ayoink = ((pkt[4]>>7)&0x1) != 0;
    u32 radius = pkt[4]&0x7;
    if (radius > 4) {
      LOG.Warning("%s: Ring packet (%d,%d) radius==%d, ignored", getName(), sx, sy, radius);
      return;
    }

    T2EventWindow & passiveEW = *mPassiveEWs[cn];
    MFM_API_ASSERT_STATE(mCircuits[0][cn].mEW == 0);
    MFM_API_ASSERT_STATE(mCircuits[0][cn].mYoinkVal < 0);
    mCircuits[0][cn].mEW = passiveEW.slotNum();
    mCircuits[0][cn].mYoinkVal = ayoink ? 1 : 0;
    passiveEW.initPassive(ourCtr, radius, cn, *this);
    if (!passiveEW.checkSiteAvailabilityForPassive()) {
      FAIL(INCOMPLETE_CODE);
    }
    if (!trySendAckPacket(cn)) {
      FAIL(INCOMPLETE_CODE);
    }
  }

  bool T2ITC::trySendAckPacket(CircuitNum cn) {
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              0xa0|mDir8,
              xitcByte1(XITC_CS_ANSWER,cn)
              );
    return trySendPacket(pb);
  }

  void T2ITC::handleHangUpPacket(T2PacketBuffer & pb) {
    LOG.Debug("%s: Enter HUP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    const char * pkt = pb.GetBuffer();
    u8 cn = pkt[1]&0xf;
    Circuit & circuit = mCircuits[1][cn]; // Pick up active side
    u32 slotnum = circuit.mEW;
    if (slotnum == 0) { // ???
      FAIL(INCOMPLETE_CODE); // VATDOOVEEDOO
    }
    T2Tile & tile = T2Tile::get();
    T2EventWindow * ew = tile.getEW(slotnum);
    MFM_API_ASSERT_NONNULL(ew);
    T2EventWindow & activeEW = *ew;

    freeActiveCircuit(cn); // We had one in this case and we're done with it
    unregisterEWRaw(activeEW); // Whether we had a circuit or not

    if (activeEW.getEWSN() != EWSN_AWACKS) 
      FAIL(INCOMPLETE_CODE); // DITTOO
    else activeEW.handleHangUp(*this);
  }

  void T2ITC::handleCacheUpdatesPacket(T2PacketBuffer & pb) {
    LOG.Debug("%s: Enter HCUP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    const char * pkt = pb.GetBuffer();
    u8 cn = pkt[1]&0xf;
    T2EventWindow & passiveEW = *mPassiveEWs[cn];
    MFM_API_ASSERT_STATE(passiveEW.getEWSN() == EWSN_PWCACHE);
    passiveEW.applyCacheUpdatesPacket(pb, *this);
  }

  void T2ITC::hangUpPassiveEW(T2EventWindow & passiveEW, CircuitNum cn) {
    MFM_API_ASSERT_ARG(passiveEW.getEWSN() == EWSN_PWCACHE);
    MFM_API_ASSERT_ARG(cn < CIRCUIT_COUNT);
    // SEND HANG UP
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              0xa0|mDir8,   /*standard+urgent to dir8*/
              xitcByte1(XITC_CS_HANGUP,cn)
              );
    if (!trySendPacket(pb)) {
      FAIL(INCOMPLETE_CODE); // DO WE HAVE TO BLOCK OR BLOW UP SOMETHING NOW?
    }

    // Free passive side circuit
    MFM_API_ASSERT_STATE(mCircuits[0][cn].mEW == passiveEW.slotNum());
    MFM_API_ASSERT_STATE(mCircuits[0][cn].mYoinkVal >= 0);
    mCircuits[0][cn].mEW = 0; // XXX really?  really really?  not max sumtin?
    mCircuits[0][cn].mYoinkVal = -1;
  }
  
  void T2ITC::handleAnswerPacket(T2PacketBuffer & pb) {
    LOG.Debug("%s: Enter HAP", getName());
    u32 len = pb.GetLength();
    MFM_API_ASSERT_ARG(len >= 2);
    const char * pkt = pb.GetBuffer();
    u8 cn = pkt[1]&0xf;
    Circuit & circuit = mCircuits[1][cn]; // Pick up active side
    u32 slotnum = circuit.mEW;
    if (slotnum == 0) { // ???
      FAIL(INCOMPLETE_CODE); // VATDOOVEEDOO
    }
    T2Tile & tile = T2Tile::get();
    T2EventWindow * ew = tile.getEW(slotnum);
    MFM_API_ASSERT_NONNULL(ew);
    T2EventWindow & activeEW = *ew;

    LOG.Message("%s %s HAP", getName(), activeEW.getName());
    if (activeEW.getEWSN() != EWSN_AWLOCKS) 
      FAIL(INCOMPLETE_CODE); // DITTOO
    else activeEW.handleACK(*this);
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
    if (len < 2) LOG.Warning("%s one byte 0x%02x packet, ignored", getName(), packet[0]);
    else {
      u32 xitc = (packet[1]>>4)&0x7;
      if (xitc == XITC_ITC_CMD) { // ITC state command
        ITCStateNumber psn = (ITCStateNumber) (packet[1]&0xf);
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
    ++mPacketsShipped;                   // another day, 
    return true;                         // 'the bird is away'
  }

  void T2ITC::setITCSN(ITCStateNumber itcsn) {
    MFM_API_ASSERT_ARG(itcsn >= 0 && itcsn < MAX_ITC_STATE_NUMBER);
    mStateNumber = itcsn;
    LOG.Message("->%s",getName());
  }

  void T2ITC::reset() {
    if (mFD >= 0) close();
    initializeFD();
    abortAllEWs();
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

  void T2ITC::abortAllEWs() {
    for (u32 i = 1; i <= MAX_EWSLOT; ++i) {
      if (mRegisteredEWs[i] != 0)
        mRegisteredEWs[i]->abortEW();
      MFM_API_ASSERT_NULL(mRegisteredEWs[i]);
    }
  }

  u32 T2ITC::registeredEWCount() const {
    return mRegisteredEWCount;
  }

  void T2ITC::initAllCircuits() {
    for (u8 i = 0; i < CIRCUIT_COUNT; ++i) {
      mActiveFree[i] = i;
      for (u8 act = 0; act <= 1; ++act) {
        mCircuits[act][i].mNumber = i;
        mCircuits[act][i].mEW = 0;  // Unassigned
        mCircuits[act][i].mYoinkVal = -1;  // Not in use
      }
    }
  }

  T2ITC::T2ITC(T2Tile& tile, Dir6 dir6, const char * name)
    : mTile(tile)
    , mDir6(dir6)
    , mDir8(mapDir6ToDir8(dir6))
    , mName(name)
    , mPacketsShipped(0u)
    , mStateNumber(ITCSN_SHUT)
    , mActiveFreeCount(CIRCUIT_COUNT)
    , mFD(-1)
    , mRegisteredEWs{ 0 }
    , mRegisteredEWCount(0)
    , mPassiveEWs{ 0 }
    , mVisibleAtomsToSend()
    , mCacheReceiveComplete(false)
    {
      for (u32 i = 0; i < CIRCUIT_COUNT; ++i)
        mPassiveEWs[i] = new T2EventWindow(mTile, i, mName);
        
      initAllCircuits();
    }

  T2ITC::~T2ITC() {
    for (u32 i = 0; i < CIRCUIT_COUNT; ++i) {
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

  T2ITCStateOps & T2ITC::getT2ITCStateOps() {
    if (mStateNumber >= T2ITCStateOps::mStateOpsArray.size())
      FAIL(ILLEGAL_STATE);
    T2ITCStateOps * ops = T2ITCStateOps::mStateOpsArray[mStateNumber];
    if (!ops)
      FAIL(ILLEGAL_STATE);
    return *ops;
  }

  bool T2ITC::allocateActiveCircuitIfNeeded(EWSlotNum ewsn, CircuitNum & circuitnum) {
    if (getITCSN() == ITCSN_SHUT) return true;  // No circuit needed
    MFM_API_ASSERT_STATE(getITCSN() == ITCSN_OPEN);
    CircuitNum ret = tryAllocateActiveCircuit();
    if (ret == ALL_CIRCUITS_BUSY) return false; // Sorry no can do
    mCircuits[1][ret].mEW = ewsn; // NOW ALLOCATED as active EW
    mCircuits[1][ret].mYoinkVal = mTile.getRandom().Between(0,1); 
    circuitnum = ret;             // NOW ALLOCATED as EW's circuit 
    return true;
  }

  CircuitNum T2ITC::tryAllocateActiveCircuit() {
    if (mActiveFreeCount==0) return ALL_CIRCUITS_BUSY;
    Random & r = mTile.getRandom();
    u8 idx = r.Between(0,mActiveFreeCount-1);
    CircuitNum ret = mActiveFree[idx];
    mActiveFree[idx] = mActiveFree[--mActiveFreeCount];
    return ret;
  }

  u32 T2ITC::activeCircuitsInUse() const {
   return CIRCUIT_COUNT - mActiveFreeCount;
  }

  s8 T2ITC::getYoinkVal(CircuitNum cn, bool forActive) const {
    MFM_API_ASSERT_ARG(cn < CIRCUIT_COUNT);
    return mCircuits[forActive ? 1 : 0][cn].mYoinkVal;
  }

  void T2ITC::freeActiveCircuit(CircuitNum cn) {
    MFM_API_ASSERT_ARG(cn < CIRCUIT_COUNT);
    MFM_API_ASSERT_STATE(mActiveFreeCount < CIRCUIT_COUNT);
    mActiveFree[mActiveFreeCount++] = cn;
    mCircuits[1][cn].mEW = 0;
    /* let it go to timeout, they're doing ping-pong
    if (activeCircuitsInUse()==0 && getITCSN() == ITCSN_FOLLOW)
      bump();
    */
  }

  void T2ITC::registerEWRaw(T2EventWindow & ew) {
    EWSlotNum sn = ew.slotNum();
    MFM_API_ASSERT_STATE(sn < MAX_EWSLOT+1);
    MFM_API_ASSERT_NULL(mRegisteredEWs[sn]);
    MFM_API_ASSERT_STATE(isVisibleUsable());
    mRegisteredEWs[sn] = &ew;
    ++mRegisteredEWCount;
  }

  void T2ITC::unregisterEWRaw(T2EventWindow & ew) {
    EWSlotNum sn = ew.slotNum();
    MFM_API_ASSERT_STATE(sn < MAX_EWSLOT+1);
    MFM_API_ASSERT_NONNULL(mRegisteredEWs[sn]);
    MFM_API_ASSERT_STATE(mRegisteredEWCount > 0);
    mRegisteredEWs[sn] = 0;
    --mRegisteredEWCount;
    if (mRegisteredEWCount == 0 && getITCSN() == ITCSN_DRAIN)
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
    }
    if (trySendPacket(pb)) return alreadyDone; // true when we just sent an empty CACHEXG
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
        ((byte0 & 0xf0) != 0xa0) ||
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
      ++count;
    }
    return count == 0; // CACHEXG w/no atoms means end
  }

  /////////////////// CUSTOM T2ITCStateOps HANDLERS

  void T2ITCStateOps_SHUT::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getCompatibilityStatus() >= 2) {
      if (itc.trySendPacket(pb)) {
        itc.setITCSN(ITCSN_DRAIN);
        itc.scheduleWait(WC_FULL);
      } else itc.scheduleWait(WC_RANDOM_SHORT);
    } else itc.scheduleWait(WC_RANDOM);
  }

  void T2ITCStateOps_SHUT::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() > ITCSN_DRAIN) itc.reset();
  }

  void T2ITCStateOps_DRAIN::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.registeredEWCount() > 0) itc.reset();
    else itc.startCacheSync();
  }

  void T2ITCStateOps_CACHEXG::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    bool sendDone = itc.sendVisibleAtoms(pb);  // True when all atoms shipped
    bool recvDone = itc.isCacheReceiveComplete(); 
    if (sendDone && recvDone) itc.setITCSN(ITCSN_OPEN);
    itc.scheduleWait(WC_RANDOM_SHORT); // For either state
  }

  void T2ITCStateOps_CACHEXG::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() < ITCSN_DRAIN) itc.reset();
    if (itc.recvCacheAtoms(pb)) itc.setCacheReceiveComplete();
  }

  void T2ITCStateOps_OPEN::timeout(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    itc.trySendPacket(pb); // Hi neighbor
    itc.scheduleWait(WC_LONG);
  }

  void T2ITCStateOps_OPEN::receive(T2ITC & itc, T2PacketBuffer & pb, TimeQueue& tq) {
    if (itc.getITCSN() < ITCSN_CACHEXG) itc.reset();
  }
}

