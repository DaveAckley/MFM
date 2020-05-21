#include "T2EventWindow.h"
#include "T2Tile.h"
#include "T2ITC.h"
#include "Packet.h"
#include "Logger.h"

namespace MFM {

  void T2EventWindow::loadSites() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    Sites & sites = tile.getSites();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = md.GetLastIndex(0);
    const u32 last = md.GetLastIndex(mRadius);

    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      mSites[sn] = sites.get(usite);
    }
  }

  void T2EventWindow::saveSites() {
    FAIL(INCOMPLETE_CODE);
  }

  bool T2EventWindow::executeEvent() {
    loadSites();
    OurT2Site & us = mSites[0];
    OurT2Atom & atom = us.GetAtom();
    u32 type = atom.GetType();
    // XXX PHONY DREG HACK
    if (type != T2_PHONY_DREG_TYPE)
      return false;
    // XXX DOUBLE PHONY: IT'S NOT EVEN DREG
    // JUST GO WEST YOUNG PERSON
    OurT2Site & west = mSites[1];
    OurT2Atom & westa = west.GetAtom();
    OurT2Atom tmp = atom;
    atom = westa;
    westa = tmp;

    return true;
  }

  void T2EventWindow::initializeEW() {
    mRadius = 0;
    mCenter = SPoint(S32_MAX,S32_MAX);
    initCircuitInfo();
    for (u32 i = 0; i < EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS); ++i)
      mSitesLive[i] = false;
  }

  void T2EventWindow::finalizeEW() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC != 0) {
        {
          T2ITC & itc = *mCircuits[i].mITC;
          if (mCircuits[i].mCircuitNum != UNALLOCATED_CIRCUIT_NUM) {
            itc.freeActiveCircuit(mCircuits[i].mCircuitNum); // XXX Need to finalize passive stuff too eventually..
            mCircuits[i].mCircuitNum = UNALLOCATED_CIRCUIT_NUM;
            mCircuits[i].mLockAcquired = false;
          }
          itc.unregisterEWRaw(*this);
        }
        mCircuits[i].mITC = 0;
      }
    }
  }

  void T2EventWindow::abortEW() {
    LOG.Message("%s: abort",getName());
    finalizeEW();
    if (isOnTQ()) remove();
    mTile.freeEW(this);
  }

  void T2EventWindow::initCircuitInfo() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      ci.mITC = 0;              // !=0 for occupancy check
      ci.mCircuitNum = UNALLOCATED_CIRCUIT_NUM;  // != UNALL_CN for lock needed
      ci.mLockAcquired = false;  // ==true for lock acquired
    }
  }

  void T2EventWindow::captureLockSequenceNumber(T2ITC& itc) {
    Dir6 dir6 = itc.mDir6;
    mLockSequenceNumber[dir6] = itc.getPacketsShipped();
  }

  void T2EventWindow::clearLockSequenceNumbers() {
    for (Dir6 dir6 = 0; dir6 < DIR6_COUNT; ++dir6)
      mLockSequenceNumber[dir6] = 0;
  }
  u32 T2EventWindow::getLockSequenceNumber(Dir6 dir6) {
    MFM_API_ASSERT_ARG(dir6 < DIR6_COUNT);
    return mLockSequenceNumber[dir6];
  }

  void T2EventWindow::handleACK(T2ITC & itc) {
    LOG.Message("%s hACK %s", getName(), itc.getName());

    MFM_API_ASSERT_STATE(getEWSN() == EWSN_AWLOCKS);
    
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == &itc) mCircuits[i].mLockAcquired = true;
    }
    if (hasAllNeededLocks()) scheduleWait(WC_NOW);
  }

  bool T2EventWindow::hasAllNeededLocks() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == 0) continue; // Not registered 
      if (mCircuits[i].mCircuitNum == UNALLOCATED_CIRCUIT_NUM) continue; // No lock needed
      if (!mCircuits[i].mLockAcquired) return false; // Opps: needed, not yet acquired
    }
    return true; // Ready to rock
  }
   
  bool T2EventWindow::isRegisteredWithAnyITCs() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC != 0) return true;
    }
    return false;
  }

  void T2EventWindow::registerWithITCIfNeeded(T2ITC & itc) {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == &itc) return; // already got it
      if (mCircuits[i].mITC == 0) {
        mCircuits[i].mITC = &itc;
        itc.registerEWRaw(*this);
        return;  // got it now
      }
    }
    FAIL(ILLEGAL_STATE);  // can't need more that MAX_CIRCUITS_PER_EW!
  }

  bool T2EventWindow::checkSiteAvailabilityForPassive() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = md.GetLastIndex(0);
    const u32 last = md.GetLastIndex(mRadius);

    std::set<T2EventWindow *> conflicts;
    // First check if region is all available, accumulating any EWs
    // this conflicts with
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      T2EventWindow * rew = tile.getSiteOwner(usite);
      if (rew != 0) {
        conflicts.insert(rew);
      } 
      if (!T2_SITE_IS_VISIBLE_OR_CACHE(usite.GetX(),usite.GetY())) {
        mSitesLive[sn] = true; // Hidden sites are good to go
        continue;
      }
    }
    
    if (conflicts.size() > 0) {
      FAIL(INCOMPLETE_CODE); // Resolve races        
      return false;
    }
    takeOwnershipOfRegion();
    return true;
  }

  bool T2EventWindow::checkSiteAvailability() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = md.GetLastIndex(0);
    const u32 last = md.GetLastIndex(mRadius);

    // First check if region is all available
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      if (tile.getSiteOwner(usite) != 0)
        return false; // It's not
      if (!T2_SITE_IS_VISIBLE_OR_CACHE(usite.GetX(),usite.GetY())) {
        mSitesLive[sn] = true; // Hidden sites are good to go
        continue;
      }
      // For cache and visible, also assess and accumulate ITC involvements
      bool isLive = true;
      for (u32 dir6 = 0; dir6 < DIR6_COUNT; ++dir6) {
        if (tile.getVisibleAndCacheRect(dir6).Contains(site)) {
          T2ITC & itc = tile.getITC(dir6);
          if (!itc.isVisibleUsable()) return false; // Sorry folks, this exhibit is closed
          registerWithITCIfNeeded(itc);
          if (T2_SITE_IS_CACHE(usite.GetX(),usite.GetY()) && !itc.isCacheUsable())
            isLive = false; // Exhibit's working but that site is currently dead
        }
      }
      if (isLive) mSitesLive[sn] = true;
    }
    return true;
  }

  bool T2EventWindow::checkCircuitAvailability() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC != 0) { // Then maybe need a circuit
        T2ITC & itc = *mCircuits[i].mITC;
        if (!itc.allocateActiveCircuitIfNeeded(mSlotNum,mCircuits[i].mCircuitNum))
          return false;   // circuit needed but none available
      }
    }
    return true;
  }

  void T2EventWindow::takeOwnershipOfRegion() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = md.GetLastIndex(0);
    const u32 last = md.GetLastIndex(mRadius);
    // Mine mine mine
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      tile.setSiteOwner(usite,this);
    }
  }

  void T2EventWindow::initPassive(SPoint ctr, u32 radius) {
    assignCenter(ctr, radius, false);
  }

  bool T2EventWindow::trySendLockRequests() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == 0) continue;
      T2ITC & itc = *mCircuits[i].mITC;
      CircuitNum cn = mCircuits[i].mCircuitNum;
      if (cn == U8_MAX) continue; // ITC is SHUT, no circuit or lock needed.
      s8 yoinkVal = itc.getYoinkVal(cn, true);
      MFM_API_ASSERT_STATE(yoinkVal >= 0); // Must be set up by now
      MFM_API_ASSERT_STATE(!mCircuits[i].mLockAcquired); // and this must be initted
      T2PacketBuffer pb;
      pb.Printf("%c%c", 0xa0|itc.mDir8, 0x90|cn);
      SPoint origin = itc.getITCOrigin();
      SPoint relEWctr = mCenter - origin;
      MFM_API_ASSERT_STATE(relEWctr.GetX() >= S8_MIN &&
                           relEWctr.GetX() <= S8_MAX &&
                           relEWctr.GetY() >= S8_MIN &&
                           relEWctr.GetY() <= S8_MAX);
      pb.Printf("%c%c",
                (s8)relEWctr.GetX(),
                (s8)relEWctr.GetY());
      u8 yoinkRad = ((yoinkVal&0x1)<<7)|mRadius;
      pb.Printf("%c",yoinkRad);
      if (!itc.trySendPacket(pb)) return false; // doh.
    }
    return true;
  }

  bool T2EventWindow::tryInitiateActiveEvent(UPoint center,u32 radius) {
    assignCenter(MakeSigned(center), radius, true); 
    if (!checkSiteAvailability()) return false;
    if (!checkCircuitAvailability()) return false;
    takeOwnershipOfRegion();
    if (isRegisteredWithAnyITCs()) {
      if (!trySendLockRequests()) return false; // WTF?
      setEWSN(EWSN_AWLOCKS);
      scheduleWait(WC_LONG);
    } else {
      setEWSN(EWSN_ABEHAVE);
      scheduleWait(WC_NOW);
    }
    return true;
  }

  void T2EventWindow::onTimeout(TimeQueue& srcTq) {
    if (mStateNum >= T2EWStateOps::mStateOpsArray.size())
      FAIL(ILLEGAL_STATE);
    T2EWStateOps * ops = T2EWStateOps::mStateOpsArray[mStateNum];
    if (!ops)
      FAIL(ILLEGAL_STATE);
    T2PacketBuffer pb;
    ops->timeout(*this, pb, srcTq);
  }

  T2EventWindow::T2EventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category)
    : mTile(tile)
    , mSlotNum(ewsn)
    , mLockSequenceNumber{ 0 }
    , mStateNum(EWSN_IDLE)
    , mCenter(0,0)
    , mRadius(0) // mRadius 0 means not in use
    , mSites() // Default ctor for the sites
    , mSitesLive()
    , mCategory(category)
  {
    LOG.Message("T2EW ctor %d/%s",ewsn,category);
  }

  /**** LATE EVENTWINDOW STATES HACKERY ****/

  /*** DEFINE STATEOPS SINGLETONS **/
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) static T2EWStateOps_##NAME singletonT2EWStateOps_##NAME;
  ALL_EW_STATES_MACRO()
#undef XX

  /*** DEFINE EWSTATENUMBER -> STATEOPS MAPPING **/
  T2EWStateOps::T2EWStateArray T2EWStateOps::mStateOpsArray = {
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) &singletonT2EWStateOps_##NAME,
  ALL_EW_STATES_MACRO()
#undef XX
    0
  };

  /*** DEFINE STUBBED-OUT STATE METHODS **/
#define YY00(NAME,FUNC) 
#define YY01(NAME,FUNC) 
#define YY10(NAME,FUNC) 
#define YY11(NAME,FUNC)                                   \
  void T2EWStateOps_##NAME::FUNC(T2EventWindow & ew,      \
                                 T2PacketBuffer & pb,     \
                                 TimeQueue& tq) {         \
    LOG.Error("%s: called stub " xstr(FUNC) " handler",   \
              ew.getName());                              \
    DIE_UNIMPLEMENTED();                                  \
  }                                                       \

#define XX(NAME,CUSTO,CUSRC,STUB,DESC) \
  YY##CUSTO##STUB(NAME,timeout)        \
  YY##CUSRC##STUB(NAME,receive)        \
  
  ALL_EW_STATES_MACRO()
#undef XX
#undef YY11
#undef YY10
#undef YY01
#undef YY00

  /*** STATE NAMES AS STRING **/
  const char * ewStateName[] = {
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) #NAME,
  ALL_EW_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getEWStateName(EWStateNumber sn) {
    if (sn >= MAX_EW_STATE_NUMBER) return "illegal";
    return ewStateName[sn];
  }

  /*** STATE DESCRIPTIONS AS STRING **/
  const char * ewStateDesc[] = {
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) DESC,
  ALL_EW_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getEWStateDescription(EWStateNumber sn) {
    if (sn >= MAX_EW_STATE_NUMBER) return "illegal";
    return ewStateDesc[sn];
  }

  const char * T2EventWindow::getName() const {
    static char buf[100];
    snprintf(buf,100,"EW%02d%s/%s",
             mSlotNum,
             getCategory(),
             getEWStateName(mStateNum));
    return buf;
  }
  /*
  void T2EventWindow::update() {
    debug("update: %p %s: %s\n",
          this,
          getEWStateName(mStateNum),
          getEWStateDescription(mStateNum));
    DIE_UNIMPLEMENTED();
  }
  */

  
  //// DEFAULT HANDLERS FOR T2EWStateOps
  void T2EWStateOps::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no timeout handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no receive handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  //// AINIT: CUSTOM STATE HANDLERS
  void T2EWStateOps_AINIT::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    // Here we have 
    ew.setEWSN(EWSN_AWLOCKS);
    ew.scheduleWait(WC_LONG);  // debug
  }

  void T2EWStateOps_AINIT::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no receive handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps_AWLOCKS::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    if (ew.hasAllNeededLocks()) {
      ew.setEWSN(EWSN_ABEHAVE);  // YOU GO DO THIS DAMN THING!
      ew.scheduleWait(WC_NOW); 
    } else {
      LOG.Error("%s on %s: TO AWLOCKS with pending locks", getStateName(), ew.getName());
      FAIL(INCOMPLETE_CODE);
    }
  }

  void T2EWStateOps_AWLOCKS::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE); // What would go here is in handleACK and its callers
  }

  void T2EWStateOps_ABEHAVE::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    if (!ew.hasAllNeededLocks()) {
      LOG.Error("%s on %s: TO AWLOCKS with pending locks", getStateName(), ew.getName());
      FAIL(INCOMPLETE_CODE);
      return;
    }
    if (ew.executeEvent()) {
      if (ew.isRegisteredWithAnyITCs())  // XXX IS THIS THE RIGHT PREDICATE?
        ew.setEWSN(EWSN_ASCACHE);
      else
        ew.setEWSN(EWSN_ACOMMIT);
      ew.scheduleWait(WC_NOW);
    } else
      FAIL(INCOMPLETE_CODE);
  }


}
