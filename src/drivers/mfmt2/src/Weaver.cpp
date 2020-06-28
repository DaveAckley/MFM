#include "Weaver.h"
#include "IWeave.h"

#include <getopt.h>
#include <time.h>

#include "FileByteSink.h"  // For STDERR
#include "Logger.h"
#include "T2Tile.h" /*for getDir6Name grr */

namespace MFM {
  void EWModel::setCS(Dir6 dir6, CircuitState state) {
    for (u32 cn = 0 ; cn < 2 ; ++cn) {
      if (mCircuitDirs[cn] == dir6) {
        mCircuitState[cn] = state;
        return;
      }
    }
    for (u32 cn = 0 ; cn < 2 ; ++cn) {
      if (mCircuitDirs[cn] == 0xff) {
        mCircuitDirs[cn] = dir6;
        mCircuitState[cn] = state;
        return;
      }
    }
    FAIL(UNREACHABLE_CODE);
  }

  CircuitState EWModel::getCS(Dir6 dir6) {
    for (u32 cn = 0; cn < 2; ++cn) {
      if (mCircuitDirs[cn] == dir6)
        return  mCircuitState[cn];
    }
    return (CircuitState) U32_MAX;
  }

  bool EWModel::isDisplayable() const {
    bool boring =
      (mStateNum == U32_MAX ||
       mStateNum == EWSN_IDLE);
    return !boring;
  }

  void EWModel::printPretty(ByteSink & bs, u32 minWidth) {
    OString64 temp;
    OString32 temp2;
    temp2.Printf("(%d,%d)+%d",
                 mCenter.GetX(),mCenter.GetY(),
                 mRadius);
    temp.Printf("%d/%s%s%02d-%s",
                mFileNum,
                mEWIdx==6 ? "a" : "p",
                mEWIdx==6 ? "" : getDir6Name(mEWIdx),
                mSlotNum,
                getEWStateName(mStateNum));
    while (temp.GetLength() < minWidth-temp2.GetLength())
      temp.Printf(".");
    temp.Printf("%s",temp2.GetZString());
    bs.Printf("%s",temp.GetZString());
  }

  EWFileSlot::EWFileSlot(u32 slotnum, u32 filenum) {
    for (u32 i = 0; i < 7; ++i) {
      mEWModelVector.push_back(EWModel(slotnum, filenum, i));
    }
    reset();
  }

  void EWFileSlot::reset() {
    for (u32 i = 0; i < mEWModelVector.size(); ++i)
      mEWModelVector[i].reset();
  }

  EWSlot::EWSlot(u32 slotnum, u32 filecount) {
    for (u32 i = 0; i < filecount; ++i) {
      mEWFileSlotVector.push_back(EWFileSlot(slotnum,i));
    }
    reset();
  }

  void EWSlot::reset() {
    for (u32 i = 0; i < mEWFileSlotVector.size(); ++i) {
      mEWFileSlotVector[i].reset();
    }
  }

  EWSlotMap::EWSlotMap(Alignment & align, u32 filecount)
    : mAlignment(align)
  {
    for (u32 i = 0; i < 32; ++i) {
      mEWSlotVector.push_back(EWSlot(i, filecount));
    }
    reset();
  }

  void EWSlotMap::reset() {
    mNextTraceLoc = 0;
    for (u32 i = 0; i < mEWSlotVector.size(); ++i) {
      mEWSlotVector[i].reset();
    }
  }

  void EWSlotMap::slewTo(u32 traceloc) {
    while (mNextTraceLoc > traceloc) {
      if (mNextTraceLoc - traceloc > traceloc)
        reset();
      else
        decrement();
    }
    while (mNextTraceLoc < traceloc) {
      if (!increment())
        break;
    }
  }

  bool EWSlotMap::modifyRaw(bool forward) {
    // We're looking at next forward.  Reverse has to skip back one
    u32 traceloc = forward ? mNextTraceLoc : mNextTraceLoc - 1;
    FileTrace * ft = mAlignment.getTraceAtLoc(traceloc);
    if (!ft) return false;
    updateForRaw(*ft,forward);
    mNextTraceLoc += forward ? 1 : -1;
    delete ft;
    return true;
  }

  EWModel * EWFileSlot::findEWModel(u32 ewindex)
  {
    if (ewindex >= mEWModelVector.size()) return 0;
    return &mEWModelVector[ewindex];
  }

  EWModel * EWSlot::findEWModel(u32 filenum, u32 ewindex)
  {
    if (filenum >= mEWFileSlotVector.size()) return 0;
    EWFileSlot & ewfileslot = mEWFileSlotVector[filenum];
    return ewfileslot.findEWModel(ewindex);
  }

  EWModel * EWSlotMap::findEWModel(u8 slotnum, u32 filenum, u32 ewindex)
  {
    if (slotnum >= mEWSlotVector.size()) return 0;
    EWSlot & ewslot = mEWSlotVector[slotnum];
    return ewslot.findEWModel(filenum, ewindex);
  }
  
  void EWSlotMap::updateForRaw(const FileTrace & ft, bool forward) {
    const Trace & trace = ft.getTrace();
    bool isEWStateChange = trace.getTraceType() == TTC_EW_StateChange;
    bool isEWAssignCenter = trace.getTraceType() == TTC_EW_AssignCenter;
    bool isEWCSChange = trace.getTraceType() == TTC_EW_CircuitStateChange;
    if (!isEWStateChange && !isEWAssignCenter && !isEWCSChange)
      return;
    FileNumber fn = ft.getWLFNumAndFilePos().first;
    const TraceAddress & addr = trace.getTraceAddress();

    const char * bad = 0;
    u8 ewindex;
    if (addr.mAddrMode == TRACE_REC_MODE_EWACTIV)
      ewindex = 6;
    else if (addr.mAddrMode >= TRACE_REC_MODE_EWPASIV_BASE &&
             addr.mAddrMode < TRACE_REC_MODE_EWPASIV_XX)
      ewindex = addr.mAddrMode - TRACE_REC_MODE_EWPASIV_BASE;
    else bad = "bad addr mode";

    if (!bad) {
      u8 slotnum = addr.mArg1;
      EWModel * ewmp = findEWModel(slotnum, fn, ewindex);
      if (!ewmp) bad = "ewm not found";
      else if (isEWStateChange) {
        EWStateNumber curState = (EWStateNumber) addr.mArg2;
        u8 nextStateByte;
        if (1 != trace.payloadRead().Scanf("%c",&nextStateByte))
          bad = "bad length";
        else {
          EWStateNumber nextState = (EWStateNumber) nextStateByte;
          EWModel & ewm = *ewmp;
          if (forward) { 
            if (ewm.mStateNum == curState || ewm.mStateNum == U32_MAX)
              ewm.mStateNum = nextState;
            else bad = "unmatched curstate";
          } else /*backward*/ {
            if (ewm.mStateNum != nextState) bad = "unmatched nextstate";
            else ewm.mStateNum = curState;
          }
          if (ewm.isDisplayable())
            insertEWModel(ewm);
          else
            removeEWModel(ewm);
        }
      } else if (isEWAssignCenter) {
        s8 cx,cy;
        u8 radius,active;
        if (4 != trace.payloadRead().Scanf("%c%c%c%c",&cx,&cy,&radius,&active))
          bad = "bad assign center match";
        else {
          EWModel & ewm = *ewmp;
          if (forward) {
            ewm.mCenter = SPoint((s32) cx, (s32) cy);
            ewm.mRadius = radius;
            ewm.mTraceLoc = ft.getTraceLoc();
          } else /* backward */ {
            ewm.mCenter = SPoint(0,0);
            ewm.mRadius = 0;
          }
          ewm.resetCircuits();
        }
      } else if (isEWCSChange) {
        u8 itcdir6, oldcs, newcs;
        if (3 != trace.payloadRead().Scanf("%c%c%c",&itcdir6,&oldcs,&newcs))
          bad = "bad CS change";
        else {
          EWModel & ewm = *ewmp;
          CircuitState cur = ewm.getCS(itcdir6);
          if (forward) {
            if (cur != U32_MAX && cur != oldcs) 
              LOG.Warning("cs fwd mismatch %d %d %d", cur, oldcs, newcs);
            else
              ewm.setCS(itcdir6, (CircuitState) newcs);
          } else /*backward*/ {
            if (cur != U32_MAX && cur != newcs) 
              LOG.Warning("cs bkd mismatch %d %d", cur, oldcs, newcs);
            else
              ewm.setCS(itcdir6, (CircuitState) oldcs);
          }
        }
      } else bad = "????";
    }

    if (bad) {
      OString32 tmp; addr.printPretty(tmp);
      LOG.Error("%d/%s %s",
                fn, tmp.GetZString(), bad);
      FAIL(ILLEGAL_STATE);
    }
  }

  void EWSlotMap::insertEWModel(EWModel& ewm) {
    EWModel * ewmp = &ewm;
    if (mEWModelOrders.find(ewmp) != mEWModelOrders.end())
      return;
    Order order = allocateFirstFreeOrder();
    mEWModelOrders[ewmp] = order;
    mOrdersEWModel[order] = ewmp;
  }

  void EWSlotMap::removeEWModel(EWModel& ewm) {
    EWModel * ewmp = &ewm;
    if (mEWModelOrders.find(ewmp) == mEWModelOrders.end())
      return;
    Order order = mEWModelOrders[ewmp];
    mEWModelOrders.erase(ewmp);
    mOrdersEWModel.erase(order);
    freeOrder(order);
  }

  EWSlotMap::Order EWSlotMap::allocateFirstFreeOrder() {
    if (mFreeOrders.size() > 0) {  // Check for previously freed
      OrderSet::iterator first = mFreeOrders.begin();
      Order ret = *first;
      mFreeOrders.erase(first);
      return ret;
    }
    // Else book a new one
    return mOrdersEWModel.size();
  }

  void EWSlotMap::freeOrder(Order order) {
    if (mFreeOrders.find(order) != mFreeOrders.end())
      LOG.Error("Double free order %d",order);
    else
      mFreeOrders.insert(order);
  }

  FileTrace::FileTrace(Trace* toOwn, u32 useLoc, WLFNumAndFilePos fp)
    : mOwnedTrace(toOwn)
    , mTraceLoc(useLoc)
    , mFPos(fp)
  {
    MFM_API_ASSERT_ARG(toOwn);
  }

  FileTrace::~FileTrace() {
    delete mOwnedTrace;
    mOwnedTrace = 0;
  }

  void FileTrace::printPretty(ByteSink& bs) {
    Trace & trace = getTrace();
    struct timespec thisTime = trace.getTimespec();
    char buf[100];
    snprintf(buf,100,"%0.3f ",UniqueTime::doubleFromTimespec(thisTime));
    bs.Printf("%s",buf);
    u32 fn = mFPos.first;
    for (u32 i = 0; i < fn; ++i) 
      bs.Printf("   ");
    bs.Printf("%d/",fn);
    trace.printPretty(bs,false);
  }

  struct timespec WeaverLogFile::getTraceEffectiveTime(Trace & trace) const {
    MFM_API_ASSERT_STATE(hasEffectiveOffset());
    UniqueTime eo(getEffectiveOffset(),0);
    UniqueTime tt(trace.mLocalTimestamp);
    UniqueTime diff = tt - eo;
    return diff.getTimespec();
  }

  FileTrace * WeaverLogFile::read(u32 useLoc, bool useOffset) {
    struct timespec zero;
    zero.tv_sec = 0;
    zero.tv_nsec = 0;
    if (useOffset) MFM_API_ASSERT_STATE(hasEffectiveOffset());
    WLFNumAndFilePos fpos(getFileNum(), getFilePos());
    Trace * trace = mTraceLogReader.read(useOffset ? getEffectiveOffset() : zero);
    if (!trace) return 0;
    return new FileTrace(trace, useLoc, fpos);
  }

  void WeaverLogFile::seek(u32 filepos) {
    if (!mFileByteSource.Seek(filepos, SEEK_SET)) {
      FAIL(ILLEGAL_STATE);
    }
  }

  void WeaverLogFile::reread() { seek(0u); }

  void WeaverLogFile::findITCSyncs(Alignment & a) {
    reread();
    FileTrace * ptr;
    while ((ptr = read(U32_MAX, false)) != 0) { // TraceLoc not used here..
      s32 tag;
      if (ptr->getTrace().reportSyncIfAny(tag))
        a.addSyncPoint(tag < 0 ? -tag : tag, this, ptr);
      else delete ptr;
    }
  }

  WeaverLogFile::WeaverLogFile(const char * filePath, u32 num)
    : mFilePath(filePath)
    , mFileNumber(num)
    , mFileByteSource(mFilePath)
    , mTraceLogReader(mFileByteSource)
    , mAverageOffset(0)
    , mOutlierDistance(10e100)
    , mHasEffectiveOffset(false)
    , mEffectiveOffset()
  { }
    
  WeaverLogFile::~WeaverLogFile() {
    mFileByteSource.Close();
  }

  EWSlotMap & Alignment::getEWSlotMap() {
    MFM_API_ASSERT_NONNULL(mEWSlotMapPtr);
    return *mEWSlotMapPtr;
  }

  bool Alignment::parseTweak(const char * arg) {
    CharBufferByteSource cbbs(arg,strlen(arg));
    u32 filenum;
    s32 usec;
    if (3 == cbbs.Scanf("%d/%d",&filenum, &usec)) {
      mWLFTweakMap[filenum] += usec;
      LOG.Message("%d/ tweak = %dus",filenum, mWLFTweakMap[filenum]);
      return true;
    }
    return false;
  }

  void Alignment::addSyncPoint(s32 sync, WeaverLogFile * file, FileTrace * evt) {
    MFM_API_ASSERT_NONNULL(file);
    MFM_API_ASSERT_NONNULL(evt);
    FileTracePtrPair pair(file,evt);
    FileTracePtrPairVector & v = mSyncUseMap[sync];
    v.push_back(pair);
  }

  Alignment::Alignment()
    : mPrintSyncMap(false)
    , mEWSlotMapPtr(0)
  { }

  Alignment::~Alignment() {
    delete mEWSlotMapPtr;
    mEWSlotMapPtr = 0;
    while (mWeaverLogFiles.size() > 0) {
      WLFandTracePtrsPair & pp = mWeaverLogFiles.back();
      WeaverLogFile* wptr = pp.first;
      FileTrace* tptr = pp.second;
      delete wptr;
      delete tptr;
      mWeaverLogFiles.pop_back();
    }
    for (SyncUseMap::iterator itr = mSyncUseMap.begin(); itr != mSyncUseMap.end(); ++itr) {
      FileTracePtrPairVector & v = itr->second;
      while (v.size()) {
        FileTracePtrPair p = v.back();
        delete p.second;
        v.pop_back();
      }
    }
  }

  u32 Alignment::logFileCount() const {
    return mWeaverLogFiles.size();
  }

  void Alignment::assignEffectiveOffsetsLaterThan(FileNumber n1) {
    MFM_API_ASSERT_NONNULL(mWeaverLogFiles[n1].first);
    const WeaverLogFile & w1 = *(mWeaverLogFiles[n1].first);
    MFM_API_ASSERT_STATE(w1.hasEffectiveOffset());
    for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
         it2 != mWeaverLogFiles.end();
         ++it2) {
      WeaverLogFile & w2 = *(it2->first);
      FileNumber n2 = w2.getFileNum();
      if (n1 == n2) continue;
      if (w2.hasEffectiveOffset()) continue;
      FileNumberPair fnp(n2,n1); // Want n2 relative to n1
      InterFileData & ifd = mDisparityMap[fnp];
      if (ifd.mDisparityCount == 0) continue;

      struct timespec eff1 = w1.getEffectiveOffset();
      double avgOffset = 1.0*ifd.mSumOfDisparities/ifd.mDisparityCount;
      LOG.Message("TWEAKING n%d %f",n2,mWLFTweakMap[n2]/1000000.0);
      avgOffset += mWLFTweakMap[n2]/1000000.0; // ADJUST OFFSET BY TWEAK
      struct timespec avgOff = UniqueTime::timespecFromDouble(avgOffset);
      struct timespec sum = UniqueTime::sum(eff1,avgOff);
      w2.setEffectiveOffset(sum);
      assignEffectiveOffsetsLaterThan(n2);
    }
  }

  void Alignment::alignLogs() {
    typedef std::set<FileNumber> NoneEarlierSet;
    NoneEarlierSet noneEarlier;
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end();
         ++it1) {
      const WeaverLogFile & w1 = *(it1->first);
      FileNumber n1 = w1.getFileNum();
      bool iStretchBackFarther = true;
      for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
           it2 != mWeaverLogFiles.end();
           ++it2) {
        const WeaverLogFile & w2 = *(it2->first);
        if (&w1 == &w2) continue;
        FileNumber n2 = w2.getFileNum();
        FileNumberPair fnp(n1,n2);
        InterFileData & ifd = mDisparityMap[fnp];
        if (ifd.mFile2StretchesBackFartherCount > 0) {
          iStretchBackFarther = false;
          break;
        }
      }
      if (iStretchBackFarther) {
        noneEarlier.insert(n1);
      }
    }
    if (noneEarlier.size() != 1) {
      LOG.Message("%d anchor files detected", noneEarlier.size());
      FAIL(INCOMPLETE_CODE);
    } else { // One
      struct timespec zero;
      zero.tv_sec = 0;
      zero.tv_nsec = 0;
      FileNumber fn = *noneEarlier.begin();
      LOG.Message("Anchor file is %d/",fn);
      WeaverLogFile & anchor = *(mWeaverLogFiles[fn].first);
      anchor.setEffectiveOffset(zero); // Oldest anchor starts consolidated relative time
      assignEffectiveOffsetsLaterThan(fn);
    }
    
  }

  void Alignment::processLogs(bool print) {
    mEWSlotMapPtr = new EWSlotMap(*this, mWeaverLogFiles.size());
    analyzeLogSync();
    if (mPrintSyncMap)
      printSyncMap(STDOUT);
    if (reportOutliers()) 
      FAIL(INCOMPLETE_CODE);
    
    alignLogs();
    reportLogs(print);
  }

  void Alignment::printSyncMap(ByteSink& bs) const {
    for (SyncUseMap::const_iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      s32 val = itr->first;
      const FileTracePtrPairVector & v = itr->second;
      if (v.size() > 1) {
        bs.Printf("%08x\n",val);
        for (FileTracePtrPairVector::const_iterator it2 = v.begin();
             it2 != v.end(); ++it2) {
          const WeaverLogFile * wlf = it2->first;
          const Trace& trace = it2->second->getTrace();
          bs.Printf("  %d/",wlf->getFileNum());
          trace.getTraceAddress().printPretty(bs);
          bs.Printf("\n");
        }
      }
    }
  }

  void Alignment::countStats(u32 fn1, u32 fn2, double delta12, double sb1, double sb2) {
    FileNumberPair fnp(fn1,fn2);
    InterFileData & ifd = mDisparityMap[fnp];
    ++ifd.mDisparityCount;
    ifd.mSumOfDisparities += delta12;
    ifd.mSumSquareOfDisparities += delta12*delta12;
    if (sb1 > sb2) ++ifd.mFile1StretchesBackFartherCount;
    if (sb2 > sb1) ++ifd.mFile2StretchesBackFartherCount;
  }

  bool Alignment::reportOutlier(u32 fn1, u32 fn2, double delta12, s32 syncVal) {
    FileNumberPair fnp(fn1,fn2);
    InterFileData & ifd = mDisparityMap[fnp];
    double avg = ifd.mSumOfDisparities / ifd.mDisparityCount;
    double var = ifd.mSumSquareOfDisparities / ifd.mDisparityCount;
    double err = delta12 - avg;
    if (err < 0) err = -err;
    if (err > 0.010) {
      LOG.Warning("WARNING: SYNC ALIAS? %08x (#%d->#%d) avg=%f, var=%f, outlier=%f, err=%f",
                  syncVal,
                  fn1,fn2,
                  avg,var,
                  delta12, err);
      return true;
    }
    return false;
  }

  bool Alignment::reportOutliers() {
    u32 count = 0;
    for (SyncUseMap::iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      s32 syncVal = itr->first;
      FileTracePtrPairVector & v = itr->second;
      if (v.size() < 2) continue;
      for (FileTracePtrPairVector::iterator it1 = v.begin();
           it1 != v.end(); ++it1) {
        for (FileTracePtrPairVector::iterator it2 = v.begin();
             it2 != v.end(); ++it2) {
          WeaverLogFile & w1 = *it1->first;
          WeaverLogFile & w2 = *it2->first;
          if (&w1 == &w2) continue;
          u32 n1 = w1.getFileNum();
          u32 n2 = w2.getFileNum();
          
          struct timespec t1 = w1.getFirstTimespec();
          struct timespec t2 = w2.getFirstTimespec();
          double delta12 =
            (t1.tv_sec - t2.tv_sec) +
            (t1.tv_nsec - t2.tv_nsec)/1000000000.0;

          if (reportOutlier(n1,n2,delta12,syncVal))
            ++count;
        }
      }
    }
    return count > 0;
  }

  void Alignment::analyzeLogSync() {
    for (WeaverLogFilePtrVector::iterator itr = mWeaverLogFiles.begin();
         itr != mWeaverLogFiles.end(); ++itr) {
      WeaverLogFile* ptr = itr->first;
      ptr->findITCSyncs(*this);
     
    }
    for (SyncUseMap::iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      FileTracePtrPairVector & v = itr->second;
      if (v.size() < 2) continue;
      for (FileTracePtrPairVector::iterator it1 = v.begin();
           it1 != v.end(); ++it1) {
        for (FileTracePtrPairVector::iterator it2 = v.begin();
             it2 != v.end(); ++it2) {
          WeaverLogFile & w1 = *it1->first;
          WeaverLogFile & w2 = *it2->first;
          if (&w1 == &w2) continue;

          u32 n1 = w1.getFileNum();
          u32 n2 = w2.getFileNum();
          
          Trace& trace1 = it1->second->getTrace();
          Trace& trace2 = it2->second->getTrace();
          struct timespec tt1 = trace1.mLocalTimestamp.getTimespec();
          struct timespec tt2 = trace2.mLocalTimestamp.getTimespec();

          struct timespec ft1 = w1.getFirstTimespec();
          struct timespec ft2 = w2.getFirstTimespec();
          double delta12 = UniqueTime::getIntervalSeconds(ft1, ft2);

          double stretchBack1 = UniqueTime::getIntervalSeconds(tt1,ft1);
          double stretchBack2 = UniqueTime::getIntervalSeconds(tt2,ft2);

          countStats(n1, n2, delta12, stretchBack1, stretchBack2);
          countStats(n2, n1, -delta12, stretchBack2, stretchBack1);

        }
      }
    }

    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      WeaverLogFile & w1 = *(it1->first);
        u32 n1 = w1.getFileNum();
        time_t sec = w1.getFirstTimespec().tv_sec;
        OString32 temp;
        temp.Printf("%s", ctime(&sec));
        temp.Chomp();
        LOG.Message("%d/->%s", n1, w1.getFilePath());
        LOG.Message("%d/ @%s", n1, temp.GetZString());
    }
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
           it2 != mWeaverLogFiles.end(); ++it2) {
        WeaverLogFile & w1 = *(it1->first);
        WeaverLogFile & w2 = *(it2->first);
        if (&w1 == &w2) continue;
        u32 n1 = w1.getFileNum();
        u32 n2 = w2.getFileNum();

        FileNumberPair fnp(n1,n2);
        const InterFileData & ifd = mDisparityMap[fnp];
        double avg = ifd.mSumOfDisparities / ifd.mDisparityCount;
        double var = ifd.mSumSquareOfDisparities / ifd.mDisparityCount;
        LOG.Message("%d/ -> %d/ n=%d, s=%f, ss=%f, avg=%f, var=%f",
                    n1, n2,
                    ifd.mDisparityCount,
                    ifd.mSumOfDisparities,
                    ifd.mSumSquareOfDisparities,
                    avg,var);
      }
    }
  }

  void Alignment::reportLogs(bool print) {
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      WeaverLogFile & w1 = *(it1->first);
      u32 n1 = w1.getFileNum();
      OString32 temp;
      UniqueTime eo(w1.getEffectiveOffset(),0);
      eo.printPretty(temp);
      LOG.Message("%d/ %s",n1,temp.GetZString());
      w1.reread(); // Set up to reread for dumping
      if (it1->second != 0) {
        delete it1->second; // Clean up leftover traces?  
        it1->second = 0;
      }

    }
    bool somebodyLive = true;
    struct timespec lastTime;
    lastTime.tv_sec = 0;
    lastTime.tv_nsec = 0;
    while (somebodyLive) {
      somebodyLive = false;
      FileTracePtrPair * pftpp = 0;
      for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
           it1 != mWeaverLogFiles.end(); ++it1) {
        FileTracePtrPair & ftpp = *it1;

        if (!pftpp) {
          if (advanceToNextTraceIfNeededAndPossible(ftpp)) {
            pftpp = &ftpp;
            somebodyLive = true;
          }
        } else {
          if (advanceToNextTraceIfNeededAndPossible(*pftpp)) {
            FileTrace * existingt = pftpp->second;
            MFM_API_ASSERT_NONNULL(existingt);
            struct timespec existing = existingt->getTrace().getTimespec();

            if (advanceToNextTraceIfNeededAndPossible(ftpp)) {
              FileTrace * newt = ftpp.second;
              MFM_API_ASSERT_NONNULL(newt);
              struct timespec challenget = newt->getTrace().getTimespec();
              if (UniqueTime(challenget,0) < UniqueTime(existing,0)) {
                pftpp = &ftpp;
                somebodyLive = true;
              }
            }
          }
        }
      }
      if (somebodyLive) {
        WeaverLogFile & wlf = *(pftpp->first);
        FileTrace * reportt = extractNextEarliestTrace(*pftpp);
        u32 mergedRecNum = mTraceLocs.size();
        mTraceLocs.push_back(reportt->getWLFNumAndFilePos());
        struct timespec thisTime = reportt->getTrace().getTimespec();
        //double seconds = UniqueTime::getIntervalSeconds(thisTime,lastTime);
        lastTime = thisTime;
        if (print) {
          printf("%d %0.3f ",
                 mergedRecNum,
                 UniqueTime::doubleFromTimespec(thisTime));
          //        STDOUT.Printf("%d ",(u32) );
          //STDOUT.Printf("%4d ",(u32) (1000*seconds));
          for (u32 i = 0; i < wlf.getFileNum(); ++i) 
            STDOUT.Printf("        ");
          STDOUT.Printf("%d/",wlf.getFileNum());
          reportt->getTrace().printPretty(STDOUT,false);
        }
        delete reportt;
      }
    }
  }
  FileTrace * Alignment::getTraceAtLoc(u32 traceloc) {
    if (traceloc >= getTraceLocCount()) return 0;
    WLFNumAndFilePos wap = mTraceLocs[traceloc];
    u32 wlfn = wap.first;
    u32 pos = wap.second;
    WeaverLogFile* wlf = mWeaverLogFiles[wlfn].first;
    wlf->seek(pos);
    return wlf->read(traceloc, true);
  }

  bool Alignment::advanceToNextTraceIfNeededAndPossible(FileTracePtrPair & ftpp) {
    if (ftpp.second == 0) {
      FileTrace * raw = ftpp.first->read(U32_MAX,true);
      if (raw) {
        ftpp.second = raw;        
      }
    }
    return (ftpp.second != 0);    // False if trace needed but no more found
  }

  struct timespec Alignment::nextEarliestTime(FileTracePtrPair & ftpp) {
    struct timespec ret;
    if (!advanceToNextTraceIfNeededAndPossible(ftpp)) {
      ret.tv_sec = S32_MAX;
      ret.tv_nsec = 0;
    } else {
      ret = ftpp.second->getTrace().getTimespec();
    }
    return ret;
  }

  FileTrace * Alignment::extractNextEarliestTrace(FileTracePtrPair & ftpp) {
    FileTrace * ret;
    if (!advanceToNextTraceIfNeededAndPossible(ftpp)) {
      FAIL(ILLEGAL_STATE);
    }
    ret = ftpp.second;
    ftpp.second = 0;
    return ret;
  }

  bool Alignment::addLogFile(const char * path) {
    // Try opening it to frontload errors
    FILE * file = fopen(path, "r");
    if (!file) {
      LOG.Error("Can't read %s: %s", path, strerror(errno));
      return false;
    }
    fclose(file);
    WLFandTracePtrsPair pp(new WeaverLogFile(path,mWeaverLogFiles.size()), 0);
    mWeaverLogFiles.push_back(pp); // Takes ownership
    return true;
  }


#define ALL_CMD_ARGS()                                          \
  XX(help,h,N,,"Print this help")                               \
  XX(interactive,i,N,,"Run interactively using curses")         \
  XX(map,m,N,,"Print the sync map")                             \
  XX(tweak,t,O,TWEAK,"Tweak file timing")                       \
  XX(version,v,N,,"Print version and exit")                     \

#if 0
  XX(log,l,O,LEVEL,"Set or increase logging")                   \
  XX(paused,p,N,,"Start up paused")                             \
  XX(trace,t,R,PATH,"Trace output to file")                     \
  XX(wincfg,w,R,PATH,"Specify window configuration file")       \

#endif

//GENERATE SHORT OPTION STRING
#define XXR ":"
#define XXN ""
#define XXO "::"
#define XX(L,S,A,V,D) #S XX##A
  static const char * CMD_LINE_SHORT_OPTIONS =
    ALL_CMD_ARGS()
    ;
#undef XXR
#undef XXN
#undef XXO  
#undef XX

//GENERATE LONG OPTION STRUCT
#define XXR required_argument
#define XXN no_argument
#define XXO optional_argument
#define XX(L,S,A,V,D) { #L, XX##A, 0, #S[0] },

  static struct option CMD_LINE_LONG_OPTIONS[] =
    {
     ALL_CMD_ARGS()
     {0,0,0,0}
    };
#undef XXR
#undef XXN
#undef XXO  
#undef XX

//GENERATE ENUM
#define XX(L,S,A,V,D) CMDARG_##L,
  enum CmdArg {
      ALL_CMD_ARGS()
      COUNT_CMDARG
  };
#undef XX


//GENERATE HELP STRING
#define XXR(v) #v " "
#define XXN(v) ""
#define XXO(v) "[" #v "] "
#define XX(L,S,A,V,D) \
  "  --" #L " " XX##A(V) "or -" #S XX##A(V) "\n\t " D "\n\n"
static const char * CMD_HELP_STRING =
  "COMMAND LINE ARGUMENTS:\n\n"
  ALL_CMD_ARGS()
  "\n"
  ;
#undef XXR
#undef XXN
#undef XXO  
#undef XX

  void Weaver::processArgs(int argc, char ** argv) {
    int c;
    int option_index;
    int fails = 0;
    int loglevel = -1;
    int wantpaused = 0;
    int wantmap = 0;
    while ((c = getopt_long(argc,argv,
                            CMD_LINE_SHORT_OPTIONS,
                            CMD_LINE_LONG_OPTIONS,
                            &option_index)) != -1) {
      switch (c) {
      case 'i':
        mInteractive = true;
        break;

      case 'p':
        wantpaused = 1;
        break;

      case 'm':
        wantmap = 1;
        break;

      case 'w':
        LOG.Message("Z");
        break;

      case 'l':
        if (optarg) {
          s32 level = LOG.ParseLevel(optarg);
          if (level < 0) {
            error("Not a logging level '%s'",optarg);
            ++fails;
          } else {
            if (loglevel >= 0)
              message("Overriding previous logging level '%s'",
                      Logger::StrLevel((Logger::Level) loglevel));
            loglevel = (Logger::Level) level;
          }
        } else ++loglevel;
        break;

      case 'h':
        printf("%s",CMD_HELP_STRING);
        exit(0);

      case 't':
        if (optarg) {
          if (!mAlignment.parseTweak(optarg)) exit(1);
        }
        break;

      case 'v':
        printf("For MFM%d.%d.%d (%s)\nBuilt on %08x at %06x by %s\n",
               MFM_VERSION_MAJOR, MFM_VERSION_MINOR, MFM_VERSION_REV,
               xstr(MFM_TREE_VERSION),
               MFM_BUILD_DATE, MFM_BUILD_TIME,
               xstr(MFM_BUILT_BY));
        exit(0);
      case '?':
        ++fails;
        break;

      default:
        abort();
      }
    }
    while (optind < argc) {
      if (!mAlignment.addLogFile(argv[optind++])) {
        LOG.Error("Bad log file");
        ++fails;
      }
    }
    if (fails) {
      fatal("%d command line problem%s",fails,fails==1?"":"s");
    }
    if (mAlignment.logFileCount() == 0)
      fatal("No log files supplied on command line");
      
    if (loglevel >= 0) {
      LOG.SetLevel(loglevel);
    }

    if (wantpaused >= 0) {
      LOG.Message("WP %d", wantpaused);
    }
    mAlignment.setPrintSyncMap(wantmap > 0);
    LOG.Message("WM %d", wantmap);
    mAlignment.processLogs(!mInteractive);
  }

  int Weaver::main(int argc, char ** argv) {
    LOG.SetByteSink(mLogBuffer);
    processArgs(argc,argv);
    if (mInteractive) {
      IWeave iw(*this);
      iw.runInteractive();
    }
    return 0;
  }

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);
    Weaver weaver;
    return weaver.main(argc,argv);
  }
}

int main(int argc, char **argv) {
  unwind_protect({
      MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
      fprintf(stderr,"Failed out of top level\n");
      exit(99);
  },{
    return MFM::MainDispatch(argc,argv);
  });
}
