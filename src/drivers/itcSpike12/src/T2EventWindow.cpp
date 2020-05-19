#include "T2EventWindow.h"
#include "T2Tile.h"
#include "Packet.h"
#include "Logger.h"

namespace MFM {

  void T2EventWindow::initializeEW() {
    mRadius = 0;
    mCenter = UPoint(U32_MAX,U32_MAX);
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
      mCircuits[i].mITC = 0;  // use mITC!=0 for occupancy check
      mCircuits[i].mCircuitNum = UNALLOCATED_CIRCUIT_NUM;  
    }
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
      SPoint site = MakeSigned(mCenter) + offset;
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
      SPoint site = MakeSigned(mCenter) + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      tile.setSiteOwner(usite,this);
    }
  }

  bool T2EventWindow::tryInitiateActiveEvent(UPoint center,u32 radius) {
    assignCenter(center, radius, true); 
    if (!checkSiteAvailability()) return false;
    if (!checkCircuitAvailability()) return false;
    takeOwnershipOfRegion();
    TimeQueue & tq = T2Tile::get().getTQ();
    if (isRegisteredWithAnyITCs()) {
      setEWSN(EWSN_AWLOCKS);
    } else {
      setEWSN(EWSN_ABEHAVE);
    }
    schedule(tq,0);   // NOW
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

  T2EventWindow::T2EventWindow(T2Tile& tile, EWSlotNum ewsn)
    : mTile(tile)
    , mSlotNum(ewsn)
    , mStateNum(EWSN_IDLE)
    , mCenter(0,0)
    , mRadius(0) // mRadius 0 means not in use
    , mSites() // Default ctor for the sites
    , mSitesLive()
  { }

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
    snprintf(buf,100,"EW%02d/%s",
             mSlotNum,
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
    //    LOG.Message("%s EW_AINIT::timeout",ew.getName());
    ew.schedule(tq,10000);  // debug
  }

  void T2EWStateOps_AWLOCKS::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE);
  }

  void T2EWStateOps_AWLOCKS::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE);
  }

}
