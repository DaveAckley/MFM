#include "T2EventWindow.h"
#include "T2Tile.h"
#include "T2ITC.h"
#include "Packet.h"
#include "Logger.h"
#include "TraceTypes.h"

#include <algorithm>

namespace MFM {

  void T2EventWindow::setEWSN(EWStateNumber ewsn) {
    assert(ewsn >= 0 && ewsn < MAX_EW_STATE_NUMBER);
    mTile.tlog(Trace(*this, TTC_EW_StateChange,"%c",ewsn));
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

  void T2EventWindow::initializeEW() {
    mRadius = 0;
    mLastSN = 0;
    mCenter = SPoint(S32_MAX,S32_MAX);
    for (u32 i = 0; i < EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS); ++i)
      mSitesLive[i] = false;
  }

  void T2ActiveEventWindow::unbindCircuitsAsNeeded() {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (ci.isBound()) 
        ci.unbindCircuit();
    }
  }

  void T2PassiveEventWindow::unbindCircuitsAsNeeded() {
    /* passive circuit is never unbound */
  }

  void T2EventWindow::finalizeEW() {
    unbindCircuitsAsNeeded();

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

#if 0
  void T2EventWindow::freeAllCircuits() {
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      Circuit & ci = mCircuits[i];
      if (ci.isAllocated()) ci.free();
    }
  }
#endif

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

  void T2ActiveEventWindow::handleAnswer(T2ITC & itc) {
    TLOG(DBG,"%s hANSWER %s", getName(), itc.getName());

    MFM_API_ASSERT_STATE(getEWSN() == EWSN_AWLOCKS);
    
    if (hasAllNeededLocks())
      scheduleWait(WC_NOW); // Bump
  }

  void T2ActiveEventWindow::sendDropsExceptTo(T2ITC * notThisOne) {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (ci.isDropNeeded()) {
        T2ITC & thisITC = ci.getITC();
        TLOG(DBG,"%s ac[%d] %s, thisitc %s",
             getName(), i, ci.getName(), thisITC.getName());
        if (&thisITC != notThisOne) {
          TLOG(DBG,"%s sending drop thisITC %p vs itc %p",
               getName(), &thisITC, notThisOne);
          trySendDropVia(thisITC);
        }
        ci.setCS(CS_DROPPED);
      }
    }
  }

  void T2ActiveEventWindow::handleBusy(T2ITC & itc) {
    TLOG(DBG,"%s hBUSY %s", getName(), itc.getName());

    if (getEWSN() != EWSN_AWLOCKS) {
      // Could probably be silent if EWSN_ADROP, but for now.
      TLOG(DBG,"%s Not AWLOCKS, ignoring BUSY",getName());
      return;
    }

    sendDropsExceptTo(&itc);
    dropActiveEW(true);
  }

  bool T2ActiveEventWindow::trySendDropVia(T2ITC & itc) {
    T2PacketBuffer pb;
    u8 sn = getSlotNum();
    pb.Printf("%c%c",
              PKT_HDR_BITMASK_STANDARD_MFM | itc.mDir8,
              xitcByte1(XITC_CS_DROP,sn)
              );
    return itc.trySendPacket(pb);
  }

  void T2ActiveEventWindow::handleHangUp(T2ITC & itc) {
    TLOG(DBG,"%s hHU %s", getName(), itc.getName());

    MFM_API_ASSERT_STATE(getEWSN() == EWSN_AWACKS);

    bool haveMoreReal = false;
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (ci.isLockHeld()) 
        haveMoreReal = true; // Not done yet
    }
    
    if (!haveMoreReal) {
      // Ugh, now scan to release the 'registration only' pseudo-circuits
      for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
        Circuit & ci = mActiveCircuits[i];
        if (ci.isBound())
          ci.unbindCircuit();
      }

      commitAndReleaseActive();
      /////// EVENT DONE
    }
  }

  void T2ActiveEventWindow::commitAndReleaseActive() {
    mTile.getStats().incrNonemptyEventsCommitted();
    saveSites();                // COMMIT ACTIVE SIDE EW!
    TLOG(DBG,"%s FREEING (%d,%d)+%d",
         getName(),
         mCenter.GetX(), mCenter.GetY(),
         mRadius);
    
    unhogEWSites(); // RELEASE CONTROL OF THE SITES!

    mTile.releaseActiveEW(*this, true);
  }

  bool T2ActiveEventWindow::hasAllNeededLocks() {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      if (!mActiveCircuits[i].isLockNeeded()) continue; // OK
      if (!mActiveCircuits[i].isLockHeld()) return false; // Opps notyet
    }
    return true; // Ready to rock
  }
   
  bool T2ActiveEventWindow::needsAnyLocks() {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) 
      if (mActiveCircuits[i].isLockNeeded()) return true;
    return false; // no locks needed
  }

  bool T2ActiveEventWindow::hasAnyLocks() {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      if (mActiveCircuits[i].isLockHeld()) return true;
    }
    return false; // We do not have no any locks
  }
   
#if 0
  bool T2EventWindow::isRegisteredWithAnyITCs() {
    FAIL(INCOMPLETE_CODE);
    for (u32 i = 0; i < MAX_CIRCUITS_PER_EW; ++i) {
      if (mActiveCircuits[i].mITC != 0) return true;
    }
    return false;
  }
#endif

  void T2ActiveEventWindow::registerWithITCIfNeeded(T2ITC & itc) {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (ci.getITCIfAny() == &itc) return; // already got it
      if (!ci.isBound()) {
        ci.bindCircuit(itc);
        ci.setMaxUnshippedSN(mLastSN);
        return;  // got it now
      }
    }
    FAIL(ILLEGAL_STATE);  // can't need more that MAX_CIRCUITS_PER_EW!
  }

  bool T2PassiveEventWindow::trySendBusy() {
    Circuit & ci = mPassiveCircuit;
    T2ITC & itc = ci.getITC();
    CircuitNum passiveCN = getSlotNum();
    T2PacketBuffer pb;
    pb.Printf("%c%c",
              PKT_HDR_BITMASK_STANDARD_MFM | itc.mDir8,
              xitcByte1(XITC_CS_BUSY,passiveCN)
              );
    return itc.trySendPacket(pb);
  }


  bool T2PassiveEventWindow::passiveWinsYoinkRace(const T2ActiveEventWindow & ew) const {
    MFM_API_ASSERT_ARG(ew.isActiveEW());     // ew is our side active initiation
    MFM_API_ASSERT_ARG(!this->isActiveEW()); // *this is passive ew for them

    const Circuit & ci = mPassiveCircuit; 
    const T2ITC & itc = ci.getITC();
    s32 themVal = ci.getYoink(); // Inbound passive yoink val
    MFM_API_ASSERT_STATE(themVal >= 0);

    // our ew must also have itc in its info, but we have to search for it
    const Circuit *aci = ew.getActiveCircuitForITCIfAny(itc);
    MFM_API_ASSERT_NONNULL(aci);
    s32 usVal = aci->getYoink();
    MFM_API_ASSERT_STATE(usVal >= 0); // Existing active yoink val
    
    bool theyAreFred = itc.isFred(); // They came in via itc, so they're fred if it is
    bool theyWin = (theyAreFred == (usVal != themVal)); // Fred always takes odds

    TLOG(DBG,"us %s (%d,%d); yoink=%d",
         ew.getName(),
         ew.getCenter().GetX(),ew.getCenter().GetY(),
         usVal);
    TLOG(DBG,"them %s (%d,%d); yoink=%d",
         getName(),
         getCenter().GetX(),getCenter().GetY(),
         themVal);
    TLOG(DBG,"YOINK %s(%s) -> %s",
         itc.getName(),
         theyAreFred ? "Fred/odds" : "Ginger/evens",
         theyWin ? "they win" : "we win"
         );
    return theyWin;
  }

  void T2PassiveEventWindow::handleDrop(T2ITC & itc) {
    TLOG(DBG,"%s: handleDrop", getName());
    if (getEWSN() == EWSN_IDLE) return;
    if (getEWSN() >= EWSN_PRESOLVE && getEWSN() <= EWSN_PWCACHE)
      unhogEWSites();
    resetPassiveEW(); // Clear gunk for next renter
  }

  static bool aEWOlder(T2ActiveEventWindow * aew1, T2ActiveEventWindow * aew2) {
    MFM_API_ASSERT_ARG(aew1 && aew2);
    return aew1->mActiveEventCountForAge < aew2->mActiveEventCountForAge;
  }

  // RETURN TRUE IF PASSIVE CONTINUES, FALSE IF WE BUSYED-OUT ON IT
  bool T2PassiveEventWindow::resolveRacesFromPassive(EWPtrSet conflicts) {
    typedef std::vector<T2ActiveEventWindow *> AEWPtrVector;
    AEWPtrVector sortable;

    // Question 1: Are any conflicts passive?
    for (EWPtrSet::iterator itr = conflicts.begin(); itr != conflicts.end(); ++itr) {
      T2EventWindow * ew = *itr;
      MFM_API_ASSERT_NONNULL(ew);
      if (ew->isPassiveEW()) { // With passive window conflicts we cannot deal
        TLOG(DBG,"RRFP-1 %s BUSYed by %s",getName(), ew->getName());
        if (!trySendBusy()) 
          FAIL(INCOMPLETE_CODE);
        return false;
      }
      T2ActiveEventWindow * aew = ew->asActiveEW();
      MFM_API_ASSERT_NONNULL(aew);
      sortable.push_back(aew);
    }

    // OK: All conflicts are active.  Now we need to sort them oldest
    // to newest according to some age info we captured.
    std::sort(sortable.begin(), sortable.end(), aEWOlder);

    // 'sortable' is now sorted by age.  Now do P4 (202002010352-notes.txt:788:)
    for (AEWPtrVector::iterator itr = sortable.begin(); itr != sortable.end(); ++itr) {
      T2EventWindow * ew = *itr;
      MFM_API_ASSERT_NONNULL(ew);

      T2ActiveEventWindow * aew = ew->asActiveEW();
      MFM_API_ASSERT_NONNULL(aew);

      if (aew->getEWSN() == EWSN_AWLOCKS) {

        // Do yoink protocol between *this (passive for them) and ew (active by us)
        bool passiveWins = passiveWinsYoinkRace(*aew);
        if (passiveWins) {
          aew->sendDropsExceptTo(0); // Send drops to all in this case?
          aew->dropActiveEW(false);
          continue;
        }
      } 

      TLOG(DBG,"RRFP-2 %s BUSYed by %s",getName(), aew->getName());
      if (!trySendBusy())  // :787: P6
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

  void T2ActiveEventWindow::dropActiveEW(bool dueToNAK) {
    if (getEWSN() != EWSN_ADROP) {
      unhogEWSites();
      setEWSN(EWSN_ADROP);
      scheduleWait(WC_HALF);
    } else {
      LOG.Warning("%s already dropped", getName());
    }
  }

  bool T2PassiveEventWindow::checkSiteAvailabilityForPassive() {
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
    
    if (conflicts.size() > 0 && !resolveRacesFromPassive(conflicts)) // Resolve races        
        return false; // passive lost the race, done

    // Passive won the race.  We hog the region on behalf of the remote active
    hogEWSites();
    loadSites(); // AND WE LOAD SITES SO THAT SAVING BACK AFTERWARDS IS SANE
    setEWSN(EWSN_PWCACHE);

    return true;
  }

  bool T2ActiveEventWindow::checkSiteAvailabilityForActive() {
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
      // For cache and visible, also assess and accumulate ITC
      // involvements.  NOTE that 'ITC registration' does NOT
      // necessarily imply 'lock needed': We register even with
      // ITCSN_SHUT, so they know about us in case they start to open.
      
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
      T2EventWindow * curew = tile.getSiteOwner(usite);
      if (!curew == !ewOrNull) {
        TLOG(ERR,"%s: Bad hog cur %s vs arg %s",
             getName(),
             curew ? curew->getName() : "0",
             ewOrNull ? ewOrNull->getName() : "0");
        FAIL(ILLEGAL_STATE);
      }
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
    tile.tlog(Trace(*this,TTC_EW_AssignCenter,"%c%c%c%c",
                    tileSite.GetX(),
                    tileSite.GetY(),
                    mRadius,
                    activeNotPassive?1:0));
  }

  void T2PassiveEventWindow::initPassive(SPoint ctr, u32 radius, bool yoink) {
    assignCenter(ctr, radius, false);

    Circuit & ci = mPassiveCircuit;
    MFM_API_ASSERT_STATE(ci.getCS() == CS_BOUND); // Nothing more
    ci.setCS(CS_RUNG);                            // Now it's more
    ci.setYoink(yoink ? 1 : 0);
    ci.setMaxUnshippedSN(-1);
  }
  
  // Send one cache update packet if possible.
  // ret > 0 end-of-update sent; ret==0 sent non-final packet; ret < 0 no buffers try again later
  s32 T2ActiveEventWindow::trySendCacheUpdatePacket(Circuit & ci) { 

    const u32 BYTES_PER_ATOM = OurT2Atom::BPA/8; // Bits Per Atom/Bits Per Byte
    const u32 BYTES_PER_SN = 1; // SiteNumber
    const u32 BYTES_PER_SITE = BYTES_PER_SN + BYTES_PER_ATOM;

    T2Tile & tile = getTile();
    OurMDist & md = tile.getMDist();
    Sites & sites = tile.getSites();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    u32 atomsShipped = 0;

    T2ITC & itc = ci.getITC();
    u8 cn = getSlotNum();

    T2PacketBuffer pb;
    pb.Printf("%c%c",
              PKT_HDR_BITMASK_STANDARD_MFM | itc.mDir8,
              xitcByte1(XITC_CS_TALK,cn)
              ); 
    s32 sn;
    for (sn = ci.getMaxUnshippedSN(); sn >= 0 ; --sn) {
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
        TLOG(DBG,"%s>>%s#%d@%d(%d,%d) 0x%04x->0x%04x",
             getName(),
             getDir6Name(itc.mDir6),
             atomsShipped,
             sn, usite.GetX(), usite.GetY(),
             onTile.GetAtom().GetType(),
             inEW.GetAtom().GetType());
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
    ci.setMaxUnshippedSN(sn);
    return atomsShipped == 0 ? 1 : 0;
  }

  bool T2EventWindow::tryReadEWAtom(ByteSource & in, u32 & sn, OurT2AtomBitVector & bv) {
    u8 tmpsn;
    if (in.Scanf("%c",&tmpsn) != 1) return false;
    TLOG(DBG,"%s CUPSN#%d mLastSN %d", getName(), tmpsn, mLastSN);
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

  void T2PassiveEventWindow::applyCacheUpdatesPacket(T2PacketBuffer & pb, T2ITC & itc) {
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
    OurMDist & md = mTile.getMDist();
    const Rect & cacheAndViz = itc.getVisibleAndCacheRect();
    while (tryReadEWAtom(cbs, sn, tmpbv)) {
      SPoint offset = md.GetPoint(sn);
      SPoint tileSite = mCenter + offset;
      if (!cacheAndViz.Contains(tileSite)) {
        TLOG(WRN,"%s sn%d(%d,%d) not accessible by %s; ignored",
             getName(),
             sn,
             tileSite.GetX(), tileSite.GetY(),
             itc.getName()
             );
        continue;
      }
      OurT2Site & site = mSites[sn];
      OurT2Atom & atom = site.GetAtom();
      
      TLOG(DBG,"RECV#%d sn%d (%d,%d) tt 0x%04x wt 0x%04x",
           count,
           sn, tileSite.GetX(), tileSite.GetY(),
           atom.GetType(),  // Atom sitting in mSites
           tmpatom.GetType()); // Inbound atom

      OurT2AtomBitVector & bv = atom.GetBits();
      bv = tmpbv;
      ++count;
    }
    
    if (sn == 0xff) { // Recvd final update packet
      commitPassiveEWAndHangUp(); 
      TLOG(DBG,"%s PASSIVE DONE",getName());
    }
  }

  void T2PassiveEventWindow::commitPassiveEWAndHangUp() {
    Circuit & ci = getPassiveCircuit();
    TLOG(DBG,"%s (%s) cPEWAHU",getName(),ci.getName());

    MFM_API_ASSERT_STATE(getEWSN()==EWSN_PWCACHE);

    CircuitNum cn = getSlotNum();

    T2ITC & itc = ci.getITC();
    itc.hangUpPassiveEW(*this,cn); // XXX handle failure

    saveSites();
    unhogEWSites();
    resetPassiveEW(); // Clear gunk for next renter
  }

  void T2PassiveEventWindow::resetPassiveEW() {
    initializeEW(); // Clear gunk for next renter
    setEWSN(EWSN_IDLE);
    getPassiveCircuit().resetCircuitForPassive();
  }

  // Send all or as much as buffers will take; false if someone blocked, true iff all all done
  bool T2ActiveEventWindow::trySendCacheUpdates() {
    bool allDone = true;
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (!ci.isLockHeld()) continue;
      if (ci.getMaxUnshippedSN() < 0) continue; // All shipped and done on this circuit
      while (true) {
        s32 ret = trySendCacheUpdatePacket(ci);
        if (ret == 0) continue;  // Sent non-final -> Go again
        if (ret < 0) allDone = false;  // Blocked, try again later
        break; // Either done or blocked on this circuit
      } 
    }
    return allDone;
  }

  bool T2ActiveEventWindow::trySendLockRequests() {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      Circuit & ci = mActiveCircuits[i];
      if (!ci.isLockNeeded()) continue;
      T2ITC & itc = ci.getITC();
      u8 yoinkBit = mTile.getRandom().CreateBits(1);
      CircuitNum cn = getSlotNum();
      T2PacketBuffer pb;
      pb.Printf("%c%c",
                PKT_HDR_BITMASK_STANDARD_MFM | itc.mDir8,
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
      pb.Printf("%c", ringByte4(yoinkBit, mRadius));
      TLOG(DBG,"%s RING %s abs(%d,%d) usrel(%d,%d) yoink=%d",
           getName(),
           getDir6Name(itc.mDir6),
           mCenter.GetX(), mCenter.GetY(),
           relEWctr.GetX(),relEWctr.GetY(),
           yoinkBit);
      if (!itc.trySendPacket(pb)) return false; // doh.
      ci.setYoink(yoinkBit);  // After we apparently succeeded
      ci.setCS(CS_RUNG);
    }
    return true;
  }

  bool T2ActiveEventWindow::tryInitiateActiveEvent(UPoint center,u32 radius) {
    assignCenter(MakeSigned(center), radius, true); 
    if (!checkSiteAvailabilityForActive()) return false;
    mActiveEventCountForAge = mTile.getStats().getNonemptyEventsStarted();
    mTile.getStats().incrNonemptyEventsStarted();
    TLOG(DBG,"%s sites available", getName());
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

  void T2EventWindow::onTimeout(TimeQueue& srcTq) {
    if (mStateNum >= T2EWStateOps::mStateOpsArray.size())
      FAIL(ILLEGAL_STATE);
    T2EWStateOps * ops = T2EWStateOps::mStateOpsArray[mStateNum];
    if (!ops)
      FAIL(ILLEGAL_STATE);
    T2PacketBuffer pb;
    if (asActiveEW())
      ops->timeout(*asActiveEW(), pb, srcTq);
    else if (asPassiveEW())
      ops->timeout(*asPassiveEW(), pb, srcTq);
    else FAIL(UNREACHABLE_CODE);
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
#define ZZ0() T2PassiveEventWindow
#define ZZ1() T2ActiveEventWindow
#define YY00(NAME,FUNC,ACT) 
#define YY01(NAME,FUNC,ACT) 
#define YY10(NAME,FUNC,ACT) 
#define YY11(NAME,FUNC,ACT)                               \
  void T2EWStateOps_##NAME::FUNC(ZZ##ACT() & ew,          \
                                 T2PacketBuffer & pb,     \
                                 TimeQueue& tq) {         \
    LOG.Error("%s: called stub " xstr(FUNC) " handler",   \
              ew.getName());                              \
    DIE_UNIMPLEMENTED();                                  \
  }                                                       \

#define XX(NAME,ACT,CUSTO,CUSRC,STUB,DESC)       \
  YY##CUSTO##STUB(NAME,timeout,ACT)              \
  YY##CUSRC##STUB(NAME,receive,ACT)              \
  
  ALL_EW_STATES_MACRO()
#undef XX
#undef YY11
#undef YY10
#undef YY01
#undef YY00
#undef ZZ1
#undef ZZ0

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
    snprintf(mNameBuf32,32,"EW%02d-%s",
             mSlotNum,
             getEWStateName(mStateNum));
    return mNameBuf32;
  }

  const char * T2ActiveEventWindow::getName() const {
    snprintf(mNameBuf32,32,"a%02d-%s",
             mSlotNum,
             getEWStateName(mStateNum));
    return mNameBuf32;
  }

  const char * T2PassiveEventWindow::getName() const {
    snprintf(mNameBuf32,32,"p%s%02d-%s",
             getCategory(),
             mSlotNum,
             getEWStateName(mStateNum));
    return mNameBuf32;
  }

  //// DEFAULT HANDLERS FOR T2EWStateOps
  void T2EWStateOps::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no timeout handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps::timeout(T2PassiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no timeout handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps::receive(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no receive handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps::receive(T2PassiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    LOG.Error("%s on %s: no receive handler", getStateName(), ew.getName());
    DIE_UNIMPLEMENTED();
  }

  //// AINIT: CUSTOM STATE HANDLERS
  void T2EWStateOps_AINIT::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    // Here we have 
    ew.setEWSN(EWSN_AWLOCKS);
    ew.scheduleWait(WC_LONG);  // debug
  }

  void T2EWStateOps_AWLOCKS::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    T2ActiveEventWindow * aew = ew.asActiveEW();
    MFM_API_ASSERT_NONNULL(aew);
    if (aew->hasAllNeededLocks()) {
      ew.setEWSN(EWSN_ABEHAVE);  // YOU GO DO THIS DAMN THING!
      ew.scheduleWait(WC_NOW); 
    } else {
      LOG.Error("%s on %s: TO AWLOCKS with pending locks", getStateName(), ew.getName());
      FAIL(INCOMPLETE_CODE);
    }
  }

  void T2EWStateOps_ADROP::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    // Timeout in ADROP means: We've already unhogged our sites, and
    // we've waited a while to keep our slotnum from being reused too
    // soon, so it's time just to free ourselves up for reuse.
    TLOG(DBG,"%s fallow period ended, freeing", ew.getName());
    ew.getTile().releaseActiveEW(ew, false);
  }

  void T2EWStateOps_ABEHAVE::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    T2ActiveEventWindow * aew = ew.asActiveEW();
    MFM_API_ASSERT_NONNULL(aew);
    if (!aew->hasAllNeededLocks()) {
      LOG.Error("%s on %s: TO AWLOCKS with pending locks", getStateName(), ew.getName());
      FAIL(INCOMPLETE_CODE);
      return;
    }
    if (aew->executeEvent()) {
      if (aew->hasAnyLocks()) {
        ew.setEWSN(EWSN_ASCACHE);
        ew.scheduleWait(WC_NOW);
      } else
        aew->commitAndReleaseActive();
    } else
      FAIL(INCOMPLETE_CODE);
  }

  void T2EWStateOps_ASCACHE::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    T2ActiveEventWindow * aew = ew.asActiveEW();
    MFM_API_ASSERT_NONNULL(aew);
    if (aew->trySendCacheUpdates()) { // True when all have been shipped to all
      ew.setEWSN(EWSN_AWACKS);
      ew.scheduleWait(WC_FULL);
    } else {
      ew.scheduleWait(WC_RANDOM_SHORT);  // Really?  Buffer space that tight?
    }
  }

#if 0 // NOT USING RECEIVE HANDLERS FOR EWs AT ALL!
  void T2EWStateOps_ASCACHE::receive(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    FAIL(INCOMPLETE_CODE); // Dealt with in handleCacheUpdates or something.
  }
#endif

  void T2EWStateOps_AWACKS::timeout(T2ActiveEventWindow & ew, T2PacketBuffer & pb, TimeQueue& tq) {
    /* XXX THIS CODE EVENTUALLY NEEDS TO EXIST and needs to perhaps
       reset the ITC or something, because we're definitely talking
       likely corruption here, if we got as far as shipping cache
       updates and our counterparties didn't respond */
    FAIL(INCOMPLETE_CODE);
  }

  /////////////////////T2ActiveEventWindow
  T2ActiveEventWindow::T2ActiveEventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category)
    : T2EventWindow(tile, ewsn, category)
    , mActiveCircuits{ *this, *this }
  { }

  const Circuit * T2ActiveEventWindow::getActiveCircuitForITCIfAny(const T2ITC & itc) const {
    for (u32 i = 0; i < CIRCUITS_PER_ACTIVE_EW; ++i) {
      const Circuit & ci = mActiveCircuits[i];
      if (ci.getITCIfAny() == &itc) return &ci;
    }
    return 0;
  }
    
  bool T2ActiveEventWindow::executeEvent() {
    loadSites();

    mTile.getStats().incrNonemptyTransitionsStarted(); // score now (in case blows up)

    OurT2Site & us = mSites[0];
    OurT2Atom & atom = us.GetAtom();
    u32 type = atom.GetType();
    // XXX PHONY DREG HACK
    if (type != T2_PHONY_DREG_TYPE)
      return false;
    // XXX DOUBLE PHONY: IT'S NOT EVEN DREG
    // JUST GO N/E/S/W IF LIVE YOUNG BEIN
    u8 ngb = atom.GetStateField(0,3)+1;
    TLOG(DBG,"%s (%d,%d)BEIN ngb %d live %d",
         getName(),
         mCenter.GetX(),mCenter.GetY(),
         ngb,
         mSitesLive[ngb]);

    if (mSitesLive[ngb]) {
      OurT2Site & west = mSites[ngb];
      OurT2Atom & westa = west.GetAtom();
      OurT2Atom tmp = westa;
      westa = atom;
      atom = tmp;
    } else {
      u8 oth = mTile.getRandom().Between(1,8);
      atom.SetStateField(0,3,oth-1);
      TLOG(DBG,"%s (%d,%d)BEIN oth %d (othlive %d)",
           getName(),
           mCenter.GetX(),mCenter.GetY(),
           oth,
           mSitesLive[oth]);
    }

#if 0
    const char * PHONY_DIRS = "\002\004\003\001";
    for (const char * dir = PHONY_DIRS; *dir != 0; ++dir) {
      u8 sn=*dir;
      if (mSitesLive[sn]) {
        OurT2Site & west = mSites[sn];
        OurT2Atom & westa = west.GetAtom();
        OurT2Atom tmp = atom;
        atom = westa;
        westa = tmp;
      }
    }
#endif
    return true;
  }

  /////////////////////T2PassiveEventWindow
  T2PassiveEventWindow::T2PassiveEventWindow(T2Tile& tile, EWSlotNum ewsn, const char * category, T2ITC& itc)
    : T2EventWindow(tile, ewsn, category)
    , mPassiveCircuit(*this)
  {
    _setEWSNRaw(EWSN_IDLE);
    mPassiveCircuit.bindCircuitForPassive(itc);
  }
  

}
