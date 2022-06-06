/* -*- C++ -*- */
#ifndef TRACELOGINFO_H
#define TRACELOGINFO_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "itype.h"
#include "T2Types.h"  /*for BPoint */
#include "ByteSink.h"

#define TRACE_DUMP_FILENAME_FMT "%08x-%08x-%02x%02x%02x.dat"
#define TRACE_DUMP_FILENAME_MATCHES 5
#define TRACE_DUMP_FILENAME_MATCH_LEN 28

namespace MFM {
  struct TraceLogInfo {
    bool tryInitFromDir(DIR * dir, struct dirent * ent) ;
    u32 mSeq;
    u32 mTag;
    u8 mRange;
    BPoint mOffset;
    u32 mBytes;
    void formatInfo(ByteSink & bs) const ;
    void reset() ;
    void tryMoveLog() const ;
  };

  struct TraceLogDirManager {
    static const u32 MAX_TRACE_SEQ_TO_KEEP = 10;

    TraceLogDirManager() ;
    const char * getTraceDir() { return "/home/t2/mfmTraces"; }
    u32 initAndManageTraceDir() ; // returns nextseq
    void tryMoveLog(u32 tag, u32 range, BPoint offset) ;
    const TraceLogInfo * getTraceLogInfoFromSeq(u32 seq) const ;
    const TraceLogInfo * getTraceLogInfoByIndex(u32 index) const ;
    TraceLogInfo mFileInfos[MAX_TRACE_SEQ_TO_KEEP];
  };
  


}
#endif /* TRACELOGINFO_H */
