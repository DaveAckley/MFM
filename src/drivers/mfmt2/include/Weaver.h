/* -*- C++ -*- */
#ifndef WEAVER_H
#define WEAVER_H

#include <stdio.h>

#include <vector>
#include <string>
#include <map>
#include <set>

#include "dirdatamacro.h"

#include "Point.h"
#include "LineTailByteSink.h"

#include "T2Constants.h"
#include "FileByteSource.h"

#include "Trace.h"
#include "T2EventWindow.h"

namespace MFM {
  struct Weaver; // FORWARD
  struct Alignment; // FORWARD
  typedef u32 FileNumber;

  typedef long FilePos;
  typedef std::pair<FileNumber,FilePos> WLFNumAndFilePos;

  typedef std::pair<FileByteSource*,u32> SourceAndNetFilePos;
  typedef std::vector<SourceAndNetFilePos> SANFPVector;

  struct FileTrace {
    FileTrace(Trace* toOwn, u32 useLoc, WLFNumAndFilePos fp) ;
    Trace & getTrace() { return * mOwnedTrace; }
    const Trace & getTrace() const { return * mOwnedTrace; }
    u32 getTraceLoc() const { return mTraceLoc; }
    WLFNumAndFilePos getWLFNumAndFilePos() const { return mFPos; }
    ~FileTrace() ;

    Trace * mOwnedTrace;
    u32 mTraceLoc;
    WLFNumAndFilePos mFPos;
    void printPretty(ByteSink& bs) ;
  };

  struct WeaverLogFile {
    WeaverLogFile(const char * filePath, u32 num) ;
    ~WeaverLogFile() ;

    ByteSource & getByteSourceAtFilePos(u32 filepos) ;
    void seek(u32 filepos) ;

    FileNumber getFileNum() const { return mFileNumber; }
    const char * getFilePath() const { return mFilePath; }
    struct timespec getTraceEffectiveTime(Trace & trace) const ;

    bool hasEffectiveOffset() const { return mHasEffectiveOffset; }
    struct timespec getEffectiveOffset() const { return mEffectiveOffset; }
    void setEffectiveOffset(struct timespec ts) {
      MFM_API_ASSERT_STATE(!mHasEffectiveOffset);
      mEffectiveOffset = ts;
      mHasEffectiveOffset = true;
    }

    void findITCSyncs(Alignment &) ;
    s32 checkForSync(Trace & evt) ;

    struct timespec getFirstTimespec() const {
      MFM_API_ASSERT_STATE(mHasFirstTimespec);
      return mFirstTimespec;
    }
    Trace * readSequential(u32 useLoc, bool useOffset) ;
    FileTrace * read(u32 useLoc, bool useOffset) ;
    void reread() ;

    FilePos getFilePos() const ;
    u32 filePosToFileIndex(u32 filepos) ;

    SourceAndNetFilePos & getSourceAndNetAtFilePos(u32 filepos) ;

    typedef std::vector<UniqueTime> TimeVector;
    TimeVector mITCSyncs[DIR6_COUNT];

    void initDirectory() ;
    void initFileOrDirectory() ;
    
    const char * mFilePath;
    const FileNumber mFileNumber;

    SANFPVector mFileByteInfoVector;
    double mAverageOffset;
    double mOutlierDistance;
    bool mHasFirstTimespec;
    struct timespec mFirstTimespec;
    bool mHasEffectiveOffset;
    struct timespec mEffectiveOffset;
    bool mIsRollingDirectory;
    typedef u32 FBIVecIndex;
    typedef std::map<FilePos,FBIVecIndex,std::greater<u32>> CutMap;
    CutMap mRollingCutPoints;
    u32 mLatestIndex;
    u32 mCurrentFilePos; // Aggregate across all indexes
  };

  struct EWModel {
    const u32 mSlotNum;
    const u32 mFileNum;
    const u8 mEWIdx; // 0..5 == pET..PNE, 6==a
    u8 mRadius;
    EWStateNumber mStateNum; // U32_MAX for unknown
    SPoint mCenter;
    u32 mTraceLoc; // U32_MAX for unset
    Dir6 mCircuitDirs[2]; // 0xff for unknown/unused
    CircuitState mCircuitState[2]; // U32_MAX for unknown/unused
    EWModel(u32 sn, u32 fn, u8 idx)
      : mSlotNum(sn)
      , mFileNum(fn)
      , mEWIdx(idx)
    { reset(); }

    void reset() {
      mRadius = 0;
      mStateNum = (EWStateNumber) U32_MAX;
      mCenter = SPoint(0,0);
      mTraceLoc = U32_MAX;
      resetCircuits();
    }

    void resetCircuits() {
      for (u32 cn = 0 ; cn < 2; ++cn) {
        mCircuitDirs[cn] = 0xff;
        mCircuitState[cn] = (CircuitState) U32_MAX;
      }
    }

    bool getCircuitInfo(u32 idx, Dir6 & dir6, CircuitState & cs) {
      if (idx >= 2) return false;
      if (mCircuitDirs[idx] == 0xff) return false;
      dir6 = mCircuitDirs[idx];
      cs = mCircuitState[idx];
      return true;
    }

    void setCS(Dir6 dir, CircuitState state) ;
    CircuitState getCS(Dir6 dir) ;

    void printPretty(ByteSink & bs, u32 minWidth) ;

    bool isDisplayable() const ;
  };

  struct EWFileSlot {
    typedef std::vector<EWModel> EWModelVector;

    EWModelVector mEWModelVector;
    EWFileSlot(u32 slotnum, u32 fileNum) ;
    void reset() ;
    EWModel * findEWModel(u32 ewindex) ;
  };

  struct EWSlot {
    typedef std::vector<EWFileSlot> EWFileSlotVector;
    EWFileSlotVector mEWFileSlotVector;
    EWSlot(u32 slotnum, u32 filecount) ;
    void reset() ;
    EWModel * findEWModel(u32 filenum, u32 ewindex) ;
  };

  struct EWSlotMap {
    typedef std::vector<EWSlot> EWSlotVector;
    EWSlotVector mEWSlotVector;
    EWSlotMap(Alignment & align, u32 filecount) ;
    void reset() ;
    bool decrement() { return modifyRaw(false); }
    bool increment() { return modifyRaw(true); }
    bool modifyRaw(bool forward) ;
    void slewTo(u32 traceloc) ;
    u32 getNextTraceLoc() const { return mNextTraceLoc; }
    EWModel * findEWModel(u8 slotnum, u32 filenum, u32 ewindex) ;
    Alignment & mAlignment;
    u32 mNextTraceLoc;
    typedef u32 Order;
    typedef std::map<Order,EWModel*> OrderEWModelMap;
    OrderEWModelMap & getOrderEWModelMap() { return mOrdersEWModel; }
  private:
    typedef std::map<EWModel*,Order> EWModelOrderMap;
    EWModelOrderMap mEWModelOrders;

    OrderEWModelMap mOrdersEWModel;

    typedef std::set<Order> OrderSet;
    OrderSet mFreeOrders;

    void insertEWModel(EWModel& ewm) ;
    void removeEWModel(EWModel& ewm) ;
    Order allocateFirstFreeOrder() ;
    void freeOrder(Order order) ;

    void updateForRaw(const FileTrace & ft, bool forward) ;
  };

  struct Alignment {
    bool mPrintSyncMap;
    EWSlotMap *mEWSlotMapPtr;
    EWSlotMap & getEWSlotMap() ;
    void setPrintSyncMap(bool b) { mPrintSyncMap = b; }

    typedef std::map<u32,s32> WLFNumToUsecMap;
    WLFNumToUsecMap mWLFTweakMap;
    bool parseTweak(const char * arg) 
;    
    typedef std::pair<WeaverLogFile*,FileTrace*> WLFandTracePtrsPair;
    typedef std::vector<WLFandTracePtrsPair> WeaverLogFilePtrVector;

    WeaverLogFilePtrVector mWeaverLogFiles;

    typedef std::vector<WLFNumAndFilePos> TraceLocs;
    TraceLocs mTraceLocs;

    typedef std::pair<WeaverLogFile*,FileTrace*> FileTracePtrPair;
    typedef std::vector<FileTracePtrPair> FileTracePtrPairVector;
    typedef std::map<s32,FileTracePtrPairVector> SyncUseMap;
    SyncUseMap mSyncUseMap;

    typedef std::pair<u32,u32> FileNumberPair;
    struct InterFileData {
      u32 mDisparityCount;
      double mSumOfDisparities;
      double mSumSquareOfDisparities;
      u32 mFile1StretchesBackFartherCount;
      u32 mFile2StretchesBackFartherCount;
    };
    typedef std::map<FileNumberPair,InterFileData> DisparityMap;
    DisparityMap mDisparityMap;

    void addSyncPoint(s32 sync, WeaverLogFile * file, FileTrace * evt) ; // TAKES OWNERSHIP OF evt BUT NOT file
    void printSyncMap(ByteSink& bs) const ;
    bool addLogFile(const char * path) ;
    void countStats(u32 fn1, u32 fn2, double delta12, double sb1, double sb2) ;
    bool reportOutliers() ;
    bool reportOutlier(u32 fn1, u32 fn2, double delta12, s32 syncVal) ;

    u32 getTraceLocCount() const { return mTraceLocs.size(); }
    FileTrace * getTraceAtLoc(u32 traceLoc) ;

    bool advanceToNextTraceIfNeededAndPossible(FileTracePtrPair & ftpp) ;
    struct timespec nextEarliestTime(FileTracePtrPair & ftpp) ;
    FileTrace * extractNextEarliestTrace(FileTracePtrPair & ftpp) ;

    u32 logFileCount() const ;
    void analyzeLogSync() ;
    void assignEffectiveOffsetsLaterThan(FileNumber fn) ;
    void alignLogs() ;
    void reportLogs(bool print) ;
    void processLogs(bool print) ;
    Alignment() ;
    ~Alignment() ;
  };

  typedef LineTailByteSink<100,80> OurLogBuffer;
  struct Weaver {
    Weaver()
      : mAlignment()
      , mInteractive(false)
    { }
    
    Alignment mAlignment;
    bool mInteractive;
    OurLogBuffer mLogBuffer;
    
    void processArgs(int argc, char ** argv) ;
    
    int main(int argc, char ** argv) ;
  };

}
#endif /* WEAVER_H */
