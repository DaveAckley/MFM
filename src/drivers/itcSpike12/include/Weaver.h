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
  struct WeaverLogFile {
    WeaverLogFile(const char * filePath, u32 num) ;
    u32 getFileNum() const { return mFileNumber; }
    const char * getFilePath() const { return mFilePath; }

    void findITCSyncs(Alignment &) ;
    s32 checkForSync(Trace & evt) ;

    struct timespec getFirstTimespec() const { return mTraceLogReader.getFirstTimespec(); }
    Trace * read() ;

    typedef std::vector<UniqueTime> TimeVector;
    TimeVector mITCSyncs[DIR6_COUNT];

    const char * mFilePath;
    const u32 mFileNumber;
    typedef std::map<UniqueTime,Trace*> TraceMap;
    TraceMap mTraceBuffer;
    FileByteSource mFileByteSource;
    TraceLogReader mTraceLogReader;
    
  };

  struct Alignment {
    typedef std::vector<WeaverLogFile*> WeaverLogFilePtrVector;
    WeaverLogFilePtrVector mWeaverLogFiles;


    typedef std::pair<WeaverLogFile*,Trace*> FileTracePtrPair;
    typedef std::vector<FileTracePtrPair> FileTracePtrPairVector;
    typedef std::map<s32,FileTracePtrPairVector> SyncUseMap;
    SyncUseMap mSyncUseMap;

    typedef std::pair<u32,u32> FileNumberPair;
    typedef std::pair<double,double> SumAndSumSquaredPair;
    typedef std::pair<u32,SumAndSumSquaredPair> CountAndSumsPair;
    typedef std::map<FileNumberPair,CountAndSumsPair> DisparityMap;
    DisparityMap mDisparityMap;

    void addSyncPoint(s32 sync, WeaverLogFile * file, Trace * evt) ; // TAKES OWNERSHIP OF evt BUT NOT file
    void printSyncMap(ByteSink& bs) const ;
    bool addLogFile(const char * path) ;
    void countStats(u32 fn1, u32 fn2, double delta12) ;

    u32 logFileCount() const ;
    void alignLogs() ;
    void reportLogs() ;
    void processLogs() ;
    ~Alignment() ;
  };

}
#endif /* WEAVER_H */
