/* -*- C++ -*- */
#ifndef WEAVER_H
#define WEAVER_H

#include <stdio.h>

#include <vector>
#include <string>
#include <set>

#include "dirdatamacro.h"

#include "Point.h"

#include "T2Constants.h"
#include "FileByteSource.h"

#include "Trace.h"
#include "T2EventWindow.h"

namespace MFM {
  struct Alignment; // FORWARD
  typedef u32 FileNumber;

  typedef std::pair<FileNumber,long> WLFNumAndFilePos;

  struct FileTrace {
    FileTrace(Trace* toOwn, WLFNumAndFilePos fp) ;
    Trace & getTrace() { return * mOwnedTrace; }
    WLFNumAndFilePos getWLFNumAndFilePos() const { return mFPos; }
    ~FileTrace() ;

    Trace * mOwnedTrace;
    WLFNumAndFilePos mFPos;
  };

  struct WeaverLogFile {
    WeaverLogFile(const char * filePath, u32 num) ;
    ~WeaverLogFile() ;
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
    EWStateNumber mStateNum;
    SPoint mCenter;
    EWModel(u32 sn, u32 fn, u8 idx)
      : mSlotNum(sn)
      , mFileNum(fn)
      , mEWIdx(idx)
      , mRadius(0)
      , mCenter(0,0)
    { }
  };

  struct EWFileSlot {
    typedef std::vector<EWModel> EWModelVector;

    EWModelVector mEWModelVector;
    EWFileSlot(u32 slotnum, u32 fileNum) ;
  };

  struct EWSlot {
    typedef std::vector<EWFileSlot> EWFileSlotVector;
    EWFileSlotVector mEWFileSlotVector;
    EWSlot(u32 slotnum, u32 filecount) ;
  };

  struct EWSlotMap {
    typedef std::vector<EWSlot> EWSlotVector;
    EWSlotVector mEWSlotVector;
    EWSlotMap(u32 filecount) ;
  };

  struct Alignment {
    bool mPrintSyncMap;
    EWSlotMap *mEWSlotMapPtr;
    void setPrintSyncMap(bool b) { mPrintSyncMap = b; }

    typedef std::map<u32,s32> WLFNumToUsecMap;
    WLFNumToUsecMap mWLFTweakMap;
    bool parseTweak(const char * arg) ;
    
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

    bool advanceToNextTraceIfNeededAndPossible(FileTracePtrPair & ftpp) ;
    struct timespec nextEarliestTime(FileTracePtrPair & ftpp) ;
    FileTrace * extractNextEarliestTrace(FileTracePtrPair & ftpp) ;

    u32 logFileCount() const ;
    void analyzeLogSync() ;
    void assignEffectiveOffsetsLaterThan(FileNumber fn) ;
    void alignLogs() ;
    void reportLogs() ;
    void processLogs() ;
    Alignment() ;
    ~Alignment() ;
  };

}
#endif /* WEAVER_H */
