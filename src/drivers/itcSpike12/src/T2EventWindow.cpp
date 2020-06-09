#include "T2EventWindow.h"
#include "T2Tile.h"
#include "T2ITC.h"
#include "Packet.h"
#include "Logger.h"
#include "TraceTypes.h"

namespace MFM {

  void T2EventWindow::setEWSN(EWStateNumber ewsn) {
    assert(ewsn >= 0 && ewsn < MAX_EW_STATE_NUMBER);
    mTile.trace(*this, TTC_EW_StateChange,"%c",ewsn);
    mStateNum = ewsn;
  }

  void T2EventWindow::loadSites() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    Sites & sites = tile.getSites();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);

    for (u32 sn = 0; sn <= mLastSN; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      mSites[sn] = sites.get(usite);
    }
  }

  void T2EventWindow::saveSites() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    Sites & sites = tile.getSites();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);

    for (u32 sn = 0; sn <= mLastSN; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);

      OurT2Site & siteOnTile = sites.get(usite);
      OurT2Atom & atomOnTile = siteOnTile.GetAtom();
      
      const OurT2Site & siteInEW = mSites[sn];
      const OurT2Atom & atomInEW = siteInEW.GetAtom();
      
      if (atomOnTile != atomInEW) 
        atomOnTile = atomInEW;

      if (sn == 0) { // Udpate base layer for ew[0] only
        OurT2Base & baseOnTile = siteOnTile.GetBase();
        const OurT2Base & baseInEW = siteInEW.GetBase();

        baseOnTile = baseInEW;
      }
    }
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
    // JUST GO WEST IF LIVE YOUNG BEIN
    if (mSitesLive[1]) {
      OurT2Site & west = mSites[1];
      OurT2Atom & westa = west.GetAtom();
      OurT2Atom tmp = atom;
      atom = westa;
      westa = tmp;
    }

    return true;
  }

  void T2EventWindow::initializeEW() {
    mRadius = 0;
    mLastSN = 0;
    mCenter = SPoint(S32_MAX,S32_MAX);
    initAllCircuitInfo();
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
            mCircuits[i].mMaxUnshippedSN = -1;
          }
          itc.unregisterEWRaw(*this);
        }
        mCircuits[i].mITC = 0;
      }
    }
    mRadius = 0;
    mLastSN = 0;
    mCenter = SPoint(S32_MAX,S32_MAX);
    setEWSN(EWSN_IDLE);
  }

  void T2EventWindow::abortEW() {
    LOG.Message("%s: abort",getName());
    finalizeEW();
    if (isOnTQ()) remove();
    mTile.freeEW(*this);
  }

  void T2EventWindow::initAllCircuitInfo() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      initCircuitInfo(ci);
    }
  }

  void T2EventWindow::initCircuitInfo(CircuitInfo & ci) {
    ci.mITC = 0;              // !=0 for occupancy check
    ci.mCircuitNum = UNALLOCATED_CIRCUIT_NUM;  // != UNALL_CN for lock needed
    ci.mLockAcquired = false;  // ==true for lock acquired
    ci.mMaxUnshippedSN = S8_MAX;      // <0 when all shipped
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
    TLOG(DBG,"%s hACK %s", getName(), itc.getName());

    MFM_API_ASSERT_STATE(getEWSN() == EWSN_AWLOCKS);
    
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == &itc) mCircuits[i].mLockAcquired = true;
    }
    if (hasAllNeededLocks()) scheduleWait(WC_NOW);
  }

  void T2EventWindow::handleHangUp(T2ITC & itc) {
    TLOG(DBG,"%s hHU %s", getName(), itc.getName());

    MFM_API_ASSERT_STATE(getEWSN() == EWSN_AWACKS);

    bool haveMoreReal = false;
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      if (ci.mITC == &itc) 
        initCircuitInfo(ci);
      else if (ci.mITC != 0 && ci.mCircuitNum != UNALLOCATED_CIRCUIT_NUM)
        haveMoreReal = true;
    }
    
    if (!haveMoreReal) {
      // Ugh, now scan to release the 'registration only' pseudo-circuits
      for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
        CircuitInfo & ci = mCircuits[i];
        if (ci.mITC != 0 && ci.mCircuitNum == UNALLOCATED_CIRCUIT_NUM) {
          ci.mITC->unregisterEWRaw(*this);
          initCircuitInfo(ci);
        }
      }
    }

    // We should just ensure everybody scans all MAX_CIRCUITS_PER_EW,
    // and then not assume the occupied ones are pressed forward, but
    MFM_API_ASSERT(MAX_CIRCUITS_PER_EW==2, UNSUPPORTED_OPERATION);
    if (mCircuits[0].mITC == 0 && mCircuits[1].mITC != 0) {
      // Swap down remaining non-zero
      CircuitInfo tmp = mCircuits[0];
      mCircuits[0] = mCircuits[1];
      mCircuits[1] = tmp;
    }

    // Now if [0].mITC == 0, We Are Done
    if (mCircuits[0].mITC == 0) {

      commitAndReleaseActive();
      /////// EVENT DONE
    }
  }

  void T2EventWindow::commitAndReleaseActive() {
    saveSites();                // COMMIT ACTIVE SIDE EW!
    TLOG(DBG,"%s FREEING (%d,%d)+%d",
         getName(),
         mCenter.GetX(), mCenter.GetY(),
         mRadius);
    
    unhogEWSites(); // RELEASE CONTROL OF THE SITES!
    if (isOnTQ()) remove();

    mTile.releaseActiveEW(*this);
  }

  bool T2EventWindow::hasAllNeededLocks() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == 0) continue; // Not registered 
      if (mCircuits[i].mCircuitNum == UNALLOCATED_CIRCUIT_NUM) continue; // No lock needed
      if (!mCircuits[i].mLockAcquired) return false; // Opps: needed, not yet acquired
    }
    return true; // Ready to rock
  }
   
  bool T2EventWindow::needsAnyLocks() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == 0) continue; // Not registered 
      if (mCircuits[i].mCircuitNum != UNALLOCATED_CIRCUIT_NUM) return true; // lock needed
    }
    return false; // no locks needed
  }

  bool T2EventWindow::hasAnyLocks() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC == 0) continue; // Not registered 
      if (mCircuits[i].mCircuitNum == UNALLOCATED_CIRCUIT_NUM) continue; // No lock needed
      if (mCircuits[i].mLockAcquired) return true; // Yes we have any locks
    }
    return false; // We have no any locks
  }
   
  bool T2EventWindow::isRegisteredWithAnyITCs() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mCircuits[i].mITC != 0) return true;
    }
    return false;
  }

  void T2EventWindow::registerWithITCIfNeeded(T2ITC & itc) {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      if (ci.mITC == &itc) return; // already got it
      if (ci.mITC == 0) {
        ci.mITC = &itc;
        itc.registerEWRaw(*this);
        ci.mMaxUnshippedSN = mLastSN;
        return;  // got it now
      }
    }
    FAIL(ILLEGAL_STATE);  // can't need more that MAX_CIRCUITS_PER_EW!
  }

  const CircuitInfo& T2EventWindow::getPassiveCircuitInfo() const
  {
    const CircuitInfo* ci = getPassiveCircuitInfoIfAny();
    MFM_API_ASSERT_NONNULL(ci);       // Have it
    return *ci;
  }

  const CircuitInfo* T2EventWindow::getPassiveCircuitInfoIfAny() const
  {
    MFM_API_ASSERT_ARG(!this->isInActiveState()); // Be passive
    const CircuitInfo & ci = mCircuits[0]; // Passive CircuitInfo always in [0]
    if (ci.mITC) return &ci;
    return ci.mITC ? &ci : 0;
  }

  bool T2EventWindow::trySendNAK() {
    MFM_API_ASSERT_ARG(!this->isInActiveState()); // We are passive
    const CircuitInfo & ci = mCircuits[0]; // Passive CircuitInfo always in [0]
    MFM_API_ASSERT_NONNULL(ci.mITC); 
    T2ITC & itc = *ci.mITC;
    CircuitNum passiveCN = ci.mCircuitNum;
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              0xa0|itc.mDir8,
              xitcByte1(XITC_CS_DROP,passiveCN)
              );
    return itc.trySendPacket(pb);
  }


  bool T2EventWindow::passiveWinsYoinkRace(const T2EventWindow & ew) const {
    MFM_API_ASSERT_ARG(ew.isInActiveState());     // ew is our side active initiation
    MFM_API_ASSERT_ARG(!this->isInActiveState()); // *this is passive ew for them
    const CircuitInfo & ci = mCircuits[0]; // Passive CircuitInfo always in [0]
    MFM_API_ASSERT_NONNULL(ci.mITC); 
    T2ITC & itc = *ci.mITC;
    CircuitNum passiveCN = ci.mCircuitNum;
    s32 themVal = itc.getYoinkVal(passiveCN,false);
    MFM_API_ASSERT_STATE(themVal >= 0);
    // our ew must also have itc in its info, but we have to search for it
    s32 usVal = -1;
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      const CircuitInfo & ci = ew.mCircuits[i];
      if (ci.mITC == &itc) {
        CircuitNum activeCN = ci.mCircuitNum;
        usVal = itc.getYoinkVal(activeCN,true);
        break;
      }
    }
    MFM_API_ASSERT_STATE(usVal >= 0);
    
    bool theyAreFred = itc.isFred(); // They came in via itc, so they're fred if it is
    bool theyWin = (theyAreFred == (usVal != themVal)); // Fred always takes odds
    TLOG(DBG,"u %s (%d,%d); t %s (%d,%d): YOINK %s(%s) us%d them%d -> %s",
              ew.getName(),
              ew.mCenter.GetX(),ew.mCenter.GetY(),
              getName(),
              mCenter.GetX(),mCenter.GetY(),
              itc.getName(),
              theyAreFred ? "Fred" : "Ginger",
              usVal,
              themVal,
              theyWin ? "they win" : "we win"
              );
    return theyWin;
  }

  bool T2EventWindow::resolveRacesFromPassive(EWPtrSet conflicts) {
    // Question 1: Are any conflicts passive?
    for (EWPtrSet::iterator itr = conflicts.begin(); itr != conflicts.end(); ++itr) {
      T2EventWindow * ew = *itr;
      MFM_API_ASSERT_NONNULL(ew);
      if (!ew->isInActiveState()) {
        FAIL(INCOMPLETE_CODE);  // Have to send a NAK once we know how
      }
    }
    // OK: All conflicts are active.  Now we need to sort them oldest
    // to newest according to some age info we captured.

    typedef std::vector<T2EventWindow *> EWPtrVector;
    EWPtrVector sortable(conflicts.begin(), conflicts.end());

    // But hrm if we just have one conflict we can try to sort out the
    // codes and the concepts without actually doing the sort..
    if (sortable.size() > 1) FAIL(INCOMPLETE_CODE);

    // Presto: 'sortable' is sorted.  Now do P4 (202002010352-notes.txt:788:)
    for (EWPtrVector::iterator itr = sortable.begin(); itr != sortable.end(); ++itr) {
      T2EventWindow * ew = *itr;
      MFM_API_ASSERT_NONNULL(ew);
      // Do yoink protocol between *this (passive for them) and ew (active by us)
      bool passiveWins = passiveWinsYoinkRace(*ew);
      if (passiveWins) {
        ew->dropActiveEW(false);
        continue;
      }
      if (!trySendNAK())  // :787: P6
        FAIL(INCOMPLETE_CODE);
      return false;
    }

    return true; // :787: P7

    // passive EW, win against ALL of the active EWs in actives?
    //
    // Each active EW is considered in turn, strictly from oldest to
    // newest in terms of when their ring packets were sent. 

    // Caller must then deal with the result:
    //
    // If the answer is false, just clean up this passive EW and be
    // done with it.  No notification is required because the active
    // sides(s) will also detect the(se) race(s) and come to the
    // analogous conclusions about who won.
    //
    // But if the answer is true
  }

  void T2EventWindow::dropActiveEW(bool dueToNAK) {
    MFM_API_ASSERT_STATE(isInActiveState());
    if (getEWSN() != EWSN_ADROP) {
      unhogEWSites();
      setEWSN(EWSN_ADROP);
      scheduleWait(WC_FULL);
    } else {
      LOG.Warning("%s already dropped", getName());
    }
  }

  bool T2EventWindow::checkSiteAvailabilityForPassive() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = 0;
    const u32 last = mLastSN;

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
      return resolveRacesFromPassive(conflicts); // Resolve races        
    }
    // THEY WIN.  WE HOG THE REGION ON THEIR BEHALF
    hogEWSites();
    setEWSN(EWSN_PWCACHE);

    return true;
  }

  bool T2EventWindow::checkSiteAvailability() {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = 0;
    const u32 last = mLastSN;

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
          if (itc.getCacheRect().Contains(site) && !itc.isCacheUsable())
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

  void T2EventWindow::hogEWSites() {
    hogOrUnhogEWSites(this);
  }

  void T2EventWindow::unhogEWSites() {
    hogOrUnhogEWSites(0);
  }

  void T2EventWindow::hogOrUnhogEWSites(T2EventWindow* ewOrNull) {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = 0;
    const u32 last = mLastSN;
    // Mine mine mine
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      UPoint usite = MakeUnsigned(site);
      tile.setSiteOwner(usite,ewOrNull);
    }
    mIsHoggingSites = ewOrNull != 0;
  }

  void T2EventWindow::assignCenter(SPoint tileSite, u32 radius, bool activeNotPassive) {
    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    MFM_API_ASSERT_STATE(mRadius == 0);
    MFM_API_ASSERT_ARG(radius != 0);
    mCenter = tileSite;
    mRadius = radius;
    mLastSN = md.GetLastIndex(mRadius);
    setEWSN(activeNotPassive ? EWSN_AINIT : EWSN_PINIT);
    tile.trace(*this,TTC_EW_AssignCenter,"%c%c%c%c",
               tileSite.GetX(),
               tileSite.GetY(),
               mRadius,
               activeNotPassive?1:0);
  }

  void T2EventWindow::initPassive(SPoint ctr, u32 radius, CircuitNum cn, T2ITC & itc) {
    assignCenter(ctr, radius, false);
    // Passive ews only ever have one cn in use.  Let it be [0]
    CircuitInfo & ci = mCircuits[0];
    MFM_API_ASSERT_STATE(ci.mITC == 0);
    MFM_API_ASSERT_STATE(ci.mCircuitNum == U8_MAX);
    
    ci.mITC = &itc;
    ci.mCircuitNum = cn;
    ci.mLockAcquired = false;
    ci.mMaxUnshippedSN = -1;
  }
  
  // Send one cache update packet if possible.
  // ret > 0 end-of-update sent; ret==0 sent non-final packet; ret < 0 no buffers try again later
  s32 T2EventWindow::trySendCacheUpdatePacket(CircuitInfo & ci) { 
    const u32 BYTES_PER_ATOM = OurT2Atom::BPA/8; // Bits Per Atom/Bits Per Byte
    const u32 BYTES_PER_SN = 1; // SiteNumber
    const u32 BYTES_PER_SITE = BYTES_PER_SN + BYTES_PER_ATOM;

    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    Sites & sites = tile.getSites();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    u32 atomsShipped = 0;

    MFM_API_ASSERT_NONNULL(ci.mITC);
    T2ITC & itc = *ci.mITC;
    CircuitNum cn = ci.mCircuitNum;

    T2PacketBuffer pb;
    pb.Printf("%c%c",
              0xa0|itc.mDir8,
              xitcByte1(XITC_CS_TALK,cn)
              ); 
    s32 sn;
    for (sn = ci.mMaxUnshippedSN; sn >= 0 ; --sn) {
      SPoint offset = md.GetPoint(sn);
      SPoint site = mCenter + offset;
      MFM_API_ASSERT_STATE(site.BoundedBy(origin,maxSite)); // Right? Active EW can't get offtile
      UPoint usite = MakeUnsigned(site);
      const OurT2Site & onTile = sites.get(usite);
      const OurT2Site & inEW = mSites[sn];
      if (onTile.GetAtom() != inEW.GetAtom()) {
        //Need to ship it
        if (pb.CanWrite() < (s32) BYTES_PER_SITE) break; //this packet is full
        pb.Printf("%c",sn); // Here comes sitenumber sn!
        const OurT2Atom & atom = inEW.GetAtom();
        const OurT2AtomBitVector & bv = atom.GetBits();
        bv.PrintBytes(pb);
        ++atomsShipped;
      }
    }
    if (sn < 0 && pb.CanWrite() > 0) {
      pb.Printf("%c",0xff); // Flag end of cache update
      atomsShipped = 0; // Fake out done logic ugh
    }
    if (!itc.trySendPacket(pb)) return -1; // Grr try again
    ci.mMaxUnshippedSN = sn;
    return atomsShipped == 0 ? 1 : 0;
  }

  bool T2EventWindow::tryReadEWAtom(ByteSource & in, u32 & sn, OurT2AtomBitVector & bv) {
    u8 tmpsn;
    if (in.Scanf("%c",&tmpsn) != 1) return false;
    if (tmpsn > mLastSN) {
      if (tmpsn == 0xff) sn = tmpsn;  // Store 0xff EOC flag
      return false;
    }
    OurT2AtomBitVector tmpbv;
    if (!tmpbv.ReadBytes(in)) return false;
    sn = tmpsn;
    bv = tmpbv;
    return true;
  }

  void T2EventWindow::applyCacheUpdatesPacket(T2PacketBuffer & pb, T2ITC & itc) {
    CharBufferByteSource cbs = pb.AsByteSource();
    u8 byte0=0, byte1=0;
    if (cbs.Scanf("%c%c",&byte0,&byte1) != 2) {
      LOG.Error("%s: Bad Talk hdr 0x%02x 0x%02x",
                getName(), byte0, byte1);
      FAIL(INCOMPLETE_CODE);
    }
    u32 sn;
    OurT2Atom tmpatom;
    OurT2AtomBitVector & tmpbv = tmpatom.GetBits();
    u32 count = 0;
    while (tryReadEWAtom(cbs, sn, tmpbv)) {
      OurT2Site & site = mSites[sn];
      OurT2Atom & atom = site.GetAtom();
      OurT2AtomBitVector & bv = atom.GetBits();
      bv = tmpbv;
      ++count;
    }
    
    if (sn == 0xff) { // Recvd final update packet
      commitPassiveEWAndHangUp(itc); 
      TLOG(DBG,"%s PASSIVE DONE",getName());
    }
  }

  void T2EventWindow::commitPassiveEWAndHangUp(T2ITC & itc) {
    TLOG(DBG,"%s (%s) cPEWAHU",getName(),itc.getName());
    MFM_API_ASSERT_STATE(getEWSN()==EWSN_PWCACHE);

    // Find circuitnumber
    CircuitNum cn = U8_MAX;
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      if (ci.mITC == &itc) {
        cn = ci.mCircuitNum;
        break;
      }
    }
    MFM_API_ASSERT_STATE(cn != U8_MAX);
    itc.hangUpPassiveEW(*this,cn); // XXX handle failure

    saveSites();
    unhogEWSites();
    initializeEW(); // Clear gunk for next renter
    setEWSN(EWSN_PINIT);
    // XXX Need more?  Really actually done on passive side, here??
  }

  // Send all or as much as buffers will take; false if someone blocked, true iff all all done
  bool T2EventWindow::trySendCacheUpdates() {
    bool allDone = true;
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      CircuitInfo & ci = mCircuits[i];
      if (ci.mITC == 0) continue;
      CircuitNum cn = ci.mCircuitNum;
      if (cn == U8_MAX) continue; // ITC is SHUT, no circuit or lock needed.
      if (ci.mMaxUnshippedSN < 0) continue; // All shipped and done on this circuit
      while (true) {
        s32 ret = trySendCacheUpdatePacket(ci);
        if (ret == 0) continue;  // Sent non-final -> Go again
        if (ret < 0) allDone = false;  // Blocked, try again later
        break; // Either done or blocked on this circuit
      } 
    }
    return allDone;
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
      MFM_API_ASSERT_STATE(mCircuits[i].mMaxUnshippedSN!=S8_MAX); // and this must be initted
      T2PacketBuffer pb;
      pb.Printf("%c%c",
                0xa0|itc.mDir8,
                xitcByte1(XITC_CS_RING,cn)
                );
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
      TLOG(DBG,"%s RING abs(%d,%d) usrel(%d,%d) yoink=%d",
                getName(),
                mCenter.GetX(), mCenter.GetY(),
                relEWctr.GetX(),relEWctr.GetY(),
                yoinkVal);
      if (!itc.trySendPacket(pb)) return false; // doh.
    }
    return true;
  }

  bool T2EventWindow::tryInitiateActiveEvent(UPoint center,u32 radius) {
    assignCenter(MakeSigned(center), radius, true); 
    if (!checkSiteAvailability()) return false;
    if (!checkCircuitAvailability()) return false;
    TLOG(DBG,"%s OWNING (%d,%d)+%d",
              getName(),
              center.GetX(), center.GetY(),
              radius);
    hogEWSites();
    if (needsAnyLocks()) {
      if (!trySendLockRequests()) return false; // WTF?
      setEWSN(EWSN_AWLOCKS);
      scheduleWait(WC_LONG);
    } else {
      setEWSN(EWSN_ABEHAVE);
      scheduleWait(WC_NOW);
    }
    return true;
  }

  bool T2EventWindow::isInActiveState() const {
    T2EWStateOps * ops = T2EWStateOps::mStateOpsArray[mStateNum];
    MFM_API_ASSERT_NONNULL(ops);
    return ops->isActive();
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
    , mNameBuf32(new char [32])
    , mLockSequenceNumber{ 0 }
    , mStateNum(EWSN_IDLE)
    , mCenter(0,0)
    , mRadius(0) // mRadius 0 means not in use
    , mLastSN(0)
    , mSites() // Default ctor for the sites
    , mSitesLive()
    , mIsHoggingSites()
    , mCategory(category)
  {
    initializeEW();
    TLOG(DBG,"T2EW ctor %d/%s",ewsn,category);
  }

  T2EventWindow::~T2EventWindow() {
    delete mNameBuf32;
    mNameBuf32 = 0;
  }

  /**** LATE EVENTWINDOW STATES HACKERY ****/

  /*** DEFINE STATEOPS SINGLETONS **/
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) static T2EWStateOps_##NAME singletonT2EWStateOps_##NAME;
  ALL_EW_STATES_MACRO()
#undef XX

  /*** DEFINE EWSTATENUMBER -> STATEOPS MAPPING **/
  T2EWStateOps::T2EWStateArray T2EWStateOps::mStateOpsArray = {
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) &singletonT2EWStateOps_##NAME,
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

#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) \
    YY##CUSTO##STUB(NAME,timeout)          \
    YY##CUSRC##STUB(NAME,receive)          \
  
  ALL_EW_STATES_MACRO()
#undef XX
#undef YY11
#undef YY10
#undef YY01
#undef YY00

  /*** STATE NAMES AS STRING **/
  const char * ewStateName[] = {
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) #NAME,
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
#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC) DESC,
  ALL_EW_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getEWStateDescription(EWStateNumber sn) {
    if (sn >= MAX_EW_STATE_NUMBER) return "illegal";
    return ewStateDesc[sn];
  }

  const char * T2EventWindow::getName() const {
    snprintf(mNameBuf32,32,"EW%02d-%s/%s",
             mSlotNum,
             getCategory(),
             getEWStateName(mStateNum));
    return mNameBuf32;
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

  void T2EWStateOps_ADROP::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE);
  }

  void T2EWStateOps_ADROP::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE); 
  }

  void T2EWStateOps_ABEHAVE::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    if (!ew.hasAllNeededLocks()) {
      LOG.Error("%s on %s: TO AWLOCKS with pending locks", getStateName(), ew.getName());
      FAIL(INCOMPLETE_CODE);
      return;
    }
    if (ew.executeEvent()) {
      if (ew.hasAnyLocks()) {
        ew.setEWSN(EWSN_ASCACHE);
        ew.scheduleWait(WC_NOW);
      } else
        ew.commitAndReleaseActive();
    } else
      FAIL(INCOMPLETE_CODE);
  }

  void T2EWStateOps_ASCACHE::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    if (ew.trySendCacheUpdates()) { // True when all have been shipped to all
      ew.setEWSN(EWSN_AWACKS);
      ew.scheduleWait(WC_FULL);
    } else {
      ew.scheduleWait(WC_RANDOM_SHORT);  // Really?  Buffer space that tight?
    }
  }

  void T2EWStateOps_ASCACHE::receive(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE); // Dealt with in handleCacheUpdates or something.
  }

  void T2EWStateOps_AWACKS::timeout(T2EventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE);
  }

}
