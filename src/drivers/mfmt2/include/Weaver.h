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

#include "T2Constants.h"
#include "FileByteSource.h"

#include "Trace.h"
#include "T2EventWindow.h"

namespace MFM {
  struct Weaver; // FORWARD
  struct Alignment; // FORWARD
  typedef u32 FileNumber;

  typedef std::pair<FileNumber,long> WLFNumAndFilePos;

  struct FileTrace {
    FileTrace(Trace* toOwn, WLFNumAndFilePos fp) ;
    Trace & getTrace() { return * mOwnedTrace; }
    const Trace & getTrace() const { return * mOwnedTrace; }
    WLFNumAndFilePos getWLFNumAndFilePos() const { return mFPos; }
    ~FileTrace() ;

    Trace * mOwnedTrace;
    WLFNumAndFilePos mFPos;
    void printPretty(ByteSink& bs) ;
  };

  struct WeaverLogFile {
    WeaverLogFile(const char * filePath, u32 num) ;
    ~WeaverLogFile() ;
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

    struct timespec getFirstTimespec() const { return mTraceLogReader.getFirstTimespec(); }
    FileTrace * read(bool useOffset) ;
    void reread() ;

    long getFilePos() const { return mFileByteSource.Tell(); }

    typedef std::vector<UniqueTime> TimeVector;
    TimeVector mITCSyncs[DIR6_COUNT];

    const char * mFilePath;
    const FileNumber mFileNumber;
    FileByteSource mFileByteSource;
    TraceLogReader mTraceLogReader;
    double mAverageOffset;
    double mOutlierDistance;
    bool mHasEffectiveOffset;
    struct timespec mEffectiveOffset;
  };

  struct EWModel {
    const u32 mSlotNum;
    const u32 mFileNum;
    const u8 mEWIdx; // 0..5 == pET..PNE, 6==a
    u8 mRadius;
    EWStateNumber mStateNum; // U32_MAX for unknwon
    SPoint mCenter;
    EWModel(u32 sn, u32 fn, u8 idx)
      : mSlotNum(sn)
      , mFileNum(fn)
      , mEWIdx(idx)
    { reset(); }

    void reset() {
      mRadius = 0;
      mStateNum = (EWStateNumber) U32_MAX;
      mCenter = SPoint(0,0);
    }

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

  struct Weaver {
    Weaver()
      : mAlignment()
      , mInteractive(false)
    { }
    
    Alignment mAlignment;
    bool mInteractive;
    
    void processArgs(int argc, char ** argv) ;
    
    int main(int argc, char ** argv) ;
  };

}
#endif /* WEAVER_H */
