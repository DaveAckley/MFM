/* -*- C++ -*- */
#ifndef WEAVER_H
#define WEAVER_H

#include <stdio.h>

#include <vector>
#include <string>
#include <set>

#include "dirdatamacro.h"

#include "T2Constants.h"
#include "FileByteSource.h"

#include "Trace.h"

namespace MFM {
  struct Alignment; // FORWARD
  typedef u32 FileNumber;
  struct WeaverLogFile {
    WeaverLogFile(const char * filePath, u32 num) ;
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
    Trace * read(bool useOffset) ;
    void reread() ;

    typedef std::vector<UniqueTime> TimeVector;
    TimeVector mITCSyncs[DIR6_COUNT];

    const char * mFilePath;
    const FileNumber mFileNumber;
    typedef std::map<UniqueTime,Trace*> TraceMap;
    TraceMap mTraceBuffer;
    FileByteSource mFileByteSource;
    TraceLogReader mTraceLogReader;
    double mAverageOffset;
    double mOutlierDistance;
    bool mHasEffectiveOffset;
    struct timespec mEffectiveOffset;
  };

  struct Alignment {
    bool mPrintSyncMap;
    void setPrintSyncMap(bool b) { mPrintSyncMap = b; }

    typedef std::map<u32,s32> WLFNumToUsecMap;
    WLFNumToUsecMap mWLFTweakMap;
    bool parseTweak(const char * arg) ;
    
    typedef std::pair<WeaverLogFile*,Trace*> WLFandTracePtrsPair;
    typedef std::vector<WLFandTracePtrsPair> WeaverLogFilePtrVector;

    WeaverLogFilePtrVector mWeaverLogFiles;


    typedef std::pair<WeaverLogFile*,Trace*> FileTracePtrPair;
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

    void addSyncPoint(s32 sync, WeaverLogFile * file, Trace * evt) ; // TAKES OWNERSHIP OF evt BUT NOT file
    void printSyncMap(ByteSink& bs) const ;
    bool addLogFile(const char * path) ;
    void countStats(u32 fn1, u32 fn2, double delta12, double sb1, double sb2) ;
    bool reportOutliers() ;
    bool reportOutlier(u32 fn1, u32 fn2, double delta12, s32 syncVal) ;

    bool advanceToNextTraceIfNeededAndPossible(FileTracePtrPair & ftpp) ;
    struct timespec nextEarliestTime(FileTracePtrPair & ftpp) ;
    Trace * extractNextEarliestTrace(FileTracePtrPair & ftpp) ;

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
