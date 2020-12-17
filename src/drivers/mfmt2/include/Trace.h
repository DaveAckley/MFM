/* -*- C++ -*- */
#ifndef TRACE_H
#define TRACE_H

#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <map>

#include "itype.h"
#include "FileByteSource.h"
#include "FileByteSink.h"
#include "CharBufferByteSource.h"
#include "OverflowableCharBufferByteSink.h"
#include "Logger.h"
#include "UniqueTime.h"
#include "T2PacketBuffer.h"

#include "dirdatamacro.h"

namespace MFM {

#define ALL_TRACE_TYPES_MACRO()                 \
  XX(,ILL,Illegal)                              \
  XX(Tile,STR,Start)                            \
  XX(Tile,TLF,TopLevelFailure)                  \
  XX(Tile,TFM,TraceFileMarker)                  \
  XX(Tile,ESS,EventStatsSnapshot)               \
  XX(Tile,STP,Stop)                             \
  XX(ITC,SCH,StateChange)                       \
  XX(ITC,PIN,PacketIn)                          \
  XX(ITC,POU,PacketOut)                         \
  XX(Log,LOG,LogTrace)                          \
  XX(EW,SCH,StateChange)                        \
  XX(EW,CTR,AssignCenter)                       \
  XX(EW,CSC,CircuitStateChange)                 \


  enum TraceTypeCode {
#define XX(TYPE,BRIEF,NAME) TTC_##TYPE##_##NAME,
   ALL_TRACE_TYPES_MACRO()
#undef XX
   TTC_COUNT

  };

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD
  struct T2EventWindow; // FORWARD

  enum TraceRecConstants {
    TRACE_REC_START_BYTE1 = 0xfa,
    TRACE_REC_START_BYTE2 = 0xde,

    TRACE_REC_MODE_ILLEGAL = 0x0,
    TRACE_REC_MODE_LOG     = 0x1,
    TRACE_REC_MODE_T2TILE  = 0x2,
    TRACE_REC_MODE_ITCDIR6 = 0x3,
    TRACE_REC_MODE_EWACTIV = 0x4,
    TRACE_REC_MODE_EWPASIV_BASE = 0x5,
    TRACE_REC_MODE_EWPASIV_ET = TRACE_REC_MODE_EWPASIV_BASE + DIR6_ET,
    TRACE_REC_MODE_EWPASIV_SE = TRACE_REC_MODE_EWPASIV_BASE + DIR6_SE,
    TRACE_REC_MODE_EWPASIV_SW = TRACE_REC_MODE_EWPASIV_BASE + DIR6_SW,
    TRACE_REC_MODE_EWPASIV_WT = TRACE_REC_MODE_EWPASIV_BASE + DIR6_WT,
    TRACE_REC_MODE_EWPASIV_NW = TRACE_REC_MODE_EWPASIV_BASE + DIR6_NW,
    TRACE_REC_MODE_EWPASIV_NE = TRACE_REC_MODE_EWPASIV_BASE + DIR6_NE,
    TRACE_REC_MODE_EWPASIV_XX = TRACE_REC_MODE_EWPASIV_BASE + DIR6_COUNT,

    TRACE_REC_FORMAT_VERSION = 0x07

  };

  struct TraceAddress {
    TraceAddress() ;
    TraceAddress(const TraceTypeCode ttc, const Logger::Level level) ;
    TraceAddress(const TraceAddress & oth) ;
    TraceAddress(const T2Tile & tile) ;
    TraceAddress(const T2ITC & itc) ;
    TraceAddress(const T2EventWindow & ew) ;
    u8 getMode() const { return mAddrMode; }
    u8 getITCDir6() const ;
    u8 getITCState() const ;

    void print(ByteSink& bs) const ;
    bool read(ByteSource& bs) ;
    void printPretty(ByteSink& bs) const ;
    const char * getModeName() const ;
    u8 mAddrMode;
    u8 mArg1;
    u8 mArg2;
  };

  struct Trace {

    template <class C>
    Trace(const C & cthing, u8 traceType, const char * format, ...)
      : Trace(cthing, traceType)
    {
      va_list ap;
      va_start(ap, format);
      payloadWrite().Vprintf(format, ap);
      va_end(ap);
    }

    const UniqueTime mLocalTimestamp;
    const u8 mTraceType;
    u8 getTraceType() const { return mTraceType; }

    struct timespec getTimespec() const {
      return mLocalTimestamp.getTimespec();
    }
    bool reportSyncIfAny(s32 & store) const ;

    Trace(const T2Tile & tile, u8 traceType)
      : mLocalTimestamp()
      , mTraceType(traceType)
      , mAddress(tile)
      , mSyncTag(0)
    { }

    Trace(const T2ITC & itc, u8 traceType)
      : mLocalTimestamp()
      , mTraceType(traceType)
      , mAddress(itc)
      , mSyncTag(0)
    { }

    Trace(const T2EventWindow & ew, u8 traceType)
      : mLocalTimestamp()
      , mTraceType(traceType)
      , mAddress(ew)
      , mSyncTag(0)
    { }

    Trace(const TraceTypeCode ttc, Logger::Level level)
      : mLocalTimestamp()
      , mTraceType(ttc)
      , mAddress(ttc,level)
      , mSyncTag(0)
    {
      if (ttc != TTC_Log_LogTrace) {
        FAIL(ILLEGAL_ARGUMENT);
      }
    }

    Trace(u8 traceType) // De novo, time is now
      : mLocalTimestamp()
      , mTraceType(traceType)
      , mSyncTag(0)
    { }

    Trace(u8 traceType, UniqueTime when) // From file
      : mLocalTimestamp(when)
      , mTraceType(traceType)
      , mSyncTag(0)
    { }

    ~Trace() { }

    void printPretty(ByteSink& bs, bool includeTime) const ;
    Trace & printf(const char * format, ...) ;

    ByteSink & payloadWrite() { return mData; }
    CharBufferByteSource payloadRead() const { return mData.AsByteSource(); }
    
    /*==0 no tag, >0 sender tag, <0 receiver tag */
    s32 getTag() const { return mSyncTag; }
    void setTag(s32 val) { mSyncTag = val; }

    bool operator<(const Trace& rhs) const {
      return mLocalTimestamp < rhs.mLocalTimestamp;
    }
    TraceAddress getTraceAddress() const { return mAddress; }
    void setTraceAddress(const TraceAddress addr) { mAddress = addr; }

  private:
    TraceAddress mAddress;
    s32 mSyncTag;
    T2PacketBuffer mData;
  };

  /**
   * An OverflowableCharBufferByteSink with a capacity of 1MB.
   */
  typedef OverflowableCharBufferByteSink<(1<<20)> OString1MB;  // Screw the +2

  struct TraceLoggerInMemory {
    OString32 mBufferName;
    OString1MB mTraceBuffers[2];
    u32 mCurBuf;
    s32 ftell() const {
      return (s32)
        (mTraceBuffers[0].GetLength() + mTraceBuffers[1].GetLength());
    }

    void dump(const char * path) ;
    
    TraceLoggerInMemory(const char * p)
      : mBufferName(p)
      , mCurBuf(0)
    {
    }

    ByteSink & getByteSink() {
      if (mTraceBuffers[mCurBuf].CanWrite() <= (1<<8)) { // G'tee 256 bytes per trace event
        mCurBuf = 1-mCurBuf;
        mTraceBuffers[mCurBuf].Reset();
      }
      return mTraceBuffers[mCurBuf];
    }
  };

  struct TraceLoggerToFile {
    static FILE * openPath(const char * path) {
      MFM_API_ASSERT_NONNULL(path);
      const char * mode = "w";
      const char * modeName = "write";
      if (*path == '+') {
        mode = "a";
        modeName = "append";
        ++path;
      }
      FILE * file = ::fopen(path,mode);
      if (!file) {
        LOG.Error("Can't %s to '%s': %s",
                  modeName,
                  path,
                  strerror(errno));
        FAIL(ILLEGAL_ARGUMENT);
      }
      return file;
    }
    TraceLoggerToFile(const char * path)
      : mFile(openPath(path))
      , mFBS(mFile)
    {
    }
    ~TraceLoggerToFile() {
      mFBS.Close();
      mFile = 0;
    }
    
    ByteSink & getByteSink() { 
      MFM_API_ASSERT_NONNULL(mFile);
      return mFBS;
    }

    s32 flush() {
      if (mFile) return fflush(mFile);
      return -EBADF;
    }

    s32 ftell() {
      if (mFile) return ::ftell(mFile);
      return -EBADF;
    }

  private:
    FILE * mFile;
    FileByteSink mFBS;
  };

  struct TraceLogger {
    TraceLoggerInMemory * mInMemory;
    TraceLoggerToFile * mToFile;
    TraceLogger(const char * path, bool inMemory)
      : mInMemory(0)
      , mToFile(0)
    {
      if (inMemory) mInMemory = new TraceLoggerInMemory(path);
      else mToFile = new TraceLoggerToFile(path);
    }

    ~TraceLogger() {
      if (mInMemory) { delete mInMemory; mInMemory = 0; }
      if (mToFile) { delete mToFile; mToFile = 0; }
    }

    s32 ftell() {
      if (mInMemory) return mInMemory->ftell();
      if (mToFile) return mToFile->ftell();
      return -EBADF;
    }

    void log(const Trace & evt) {
      if (mInMemory) { log(mInMemory->getByteSink(),evt); }
      if (mToFile) { log(mToFile->getByteSink(),evt); }
    }

    void dump(const char * path) {
      if (mInMemory) mInMemory->dump(path);
    }

    static void log(ByteSink & bs, const Trace & evt);
  };



  struct TraceLogReader {
    static Trace * read(ByteSource & bs,
                        struct timespec basetime,     // Subtracts this
                        struct timespec timeoffset) ; // then adds this
  };

}

#endif /* TRACE_H */
