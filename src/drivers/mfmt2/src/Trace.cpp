#include "Trace.h"
#include "TraceTypes.h"
#include "T2Tile.h"
#include "T2ITC.h"
#include "T2EventWindow.h"
#include "Circuit.h"
#include "T2Utils.h" /* for printComma */
               
namespace MFM {
  ///// STATIC MEMBER DEFINITIONS

  ////// TraceAddress
  TraceAddress::TraceAddress()
    : mAddrMode(TRACE_REC_MODE_ILLEGAL) , mArg1(0) , mArg2(0) { }
  TraceAddress::TraceAddress(const TraceTypeCode ttc, const Logger::Level level)
    : mAddrMode(TRACE_REC_MODE_LOG) , mArg1(ttc) , mArg2(level) { }
  TraceAddress::TraceAddress(const TraceAddress & oth)
    : mAddrMode(oth.mAddrMode) , mArg1(oth.mArg1) , mArg2(oth.mArg2) { }
  TraceAddress::TraceAddress(const T2Tile & tile)
    : mAddrMode(TRACE_REC_MODE_T2TILE) , mArg1(0) , mArg2(0) { }
  TraceAddress::TraceAddress(const T2ITC & itc)
    : mAddrMode(TRACE_REC_MODE_ITCDIR6) , mArg1(itc.mDir6) , mArg2(itc.getITCSN()) { }
  TraceAddress::TraceAddress(const T2EventWindow & ew) {
    if (ew.isActiveEW()) {
      mAddrMode = TRACE_REC_MODE_EWACTIV;
      mArg1 = ew.getSlotNum();
      mArg2 = ew.getEWSN();
    } else {
      const T2PassiveEventWindow * pew = ew.asPassiveEW();
      mAddrMode = pew ?
        (TRACE_REC_MODE_EWPASIV_BASE +
         pew->getPassiveCircuit().getITC().mDir6) :
        TRACE_REC_MODE_EWPASIV_XX;
      mArg1 = pew ? pew->getSlotNum() : 0xff;
      mArg2 = ew.getEWSN();
    }
  }

  void TraceAddress::print(ByteSink& bs) const {
    bs.Printf("%c%c%c", mAddrMode, mArg1, mArg2);
  }

  bool TraceAddress::read(ByteSource& bs) {
    TraceAddress tmp;
    if (3 != bs.Scanf("%c%c%c", &tmp.mAddrMode, &tmp.mArg1, &tmp.mArg2))
      return false;
    *this = tmp;
    return true;
  }

  const char * TraceAddress::getModeName() const {
    switch (mAddrMode) {
    default:
    case TRACE_REC_MODE_ILLEGAL:    return "illegal";
    case TRACE_REC_MODE_T2TILE :    return "t";
    case TRACE_REC_MODE_ITCDIR6:    return "i";
    case TRACE_REC_MODE_EWACTIV:    return "a";
    case TRACE_REC_MODE_EWPASIV_ET: return "pET";
    case TRACE_REC_MODE_EWPASIV_SE: return "pSE";
    case TRACE_REC_MODE_EWPASIV_SW: return "pSW";
    case TRACE_REC_MODE_EWPASIV_WT: return "pWT";
    case TRACE_REC_MODE_EWPASIV_NW: return "pNW";
    case TRACE_REC_MODE_EWPASIV_NE: return "pNE";
    case TRACE_REC_MODE_EWPASIV_XX: return "pXX";
    case TRACE_REC_MODE_LOG:        return "l";
    }
  }

  void TraceAddress::printPretty(ByteSink& bs) const {
    bs.Printf("%s", getModeName());
    switch (mAddrMode) {
    default:
    case TRACE_REC_MODE_ILLEGAL: break;
    case TRACE_REC_MODE_T2TILE : break;
    case TRACE_REC_MODE_ITCDIR6:
      bs.Printf("%s",getDir6Name(mArg1));
      bs.Printf(":%s",getITCStateName((ITCStateNumber) mArg2));
      break;
    case TRACE_REC_MODE_EWACTIV:
    case TRACE_REC_MODE_EWPASIV_ET:
    case TRACE_REC_MODE_EWPASIV_SE:
    case TRACE_REC_MODE_EWPASIV_SW:
    case TRACE_REC_MODE_EWPASIV_WT:
    case TRACE_REC_MODE_EWPASIV_NW:
    case TRACE_REC_MODE_EWPASIV_NE:
    case TRACE_REC_MODE_EWPASIV_XX:
      bs.Printf("%02d",mArg1);
      bs.Printf("-%s",getEWStateName((EWStateNumber) mArg2));
      break;
    case TRACE_REC_MODE_LOG:
      if (mArg1 != TTC_Log_LogTrace)
        bs.Printf("%d?",mArg1);
      bs.Printf("%s",Logger::StrLevel((Logger::Level) mArg2));
      break;
    }
  }

  ///// 'DRIVE BY' CTORS
  

  Trace& Trace::printf(const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    payloadWrite().Vprintf(format, ap);
    va_end(ap);
    return *this;
  }

  void Trace::printPretty(ByteSink & bs, bool includeTime) const {
    if (includeTime) {
      mLocalTimestamp.printPretty(bs);
      bs.Printf(" ");
    }
    getTraceAddress().printPretty(bs);
    bs.Printf(" ");

    if (mTraceType == TTC_Tile_Start) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      s32 version = -1;
      cbbs.Scanf("%D",&version);
      bs.Printf(" Trace Format Version %d\n",version);
      return;
    } 

    if (mTraceType == TTC_Tile_TopLevelFailure) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      bs.Printf(" Failed to top: %<\n",&cbbs);
      return;
    }

    if (mTraceType == TTC_Tile_TraceFileMarker) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      s32 syncTag = 0;
      cbbs.Scanf("%l",&syncTag);
      if (syncTag < 0)
        bs.Printf(" File sync mark -?%08x\n", -syncTag);
      else
        bs.Printf(" File sync mark +?%08x\n", syncTag);
      return;
    }
    
    if (mTraceType == TTC_Tile_EventStatsSnapshot) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      T2TileStats stats;
      if (!stats.loadRaw(cbbs)) bs.Printf("??stats load failed");
      else {
        u32 secs = stats.getAgeSeconds();
        bs.Printf(" sec=");
        printComma(secs,bs);
        bs.Printf("; tot=");
        printComma(stats.getEventsConsidered(),bs);
        bs.Printf("; emp=");
        printComma(stats.getEmptyEventsCommitted(),bs);
        bs.Printf("; occ=");
        printComma(stats.getNonemptyEventsCommitted(),bs);
        bs.Printf("; estAER=%f", stats.getEstAER());
      }
      bs.Printf("\n");
      return;
    } 

    if (mTraceType == TTC_Tile_Stop) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      bs.Printf(" TRACE STOP ");
      bs.Printf("\n");
      
      return;
    } 

    if (mTraceType == TTC_ITC_StateChange) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      u8 newstate;
      if (1 == cbbs.Scanf("%c",&newstate)) bs.Printf("-> %s\n", getITCStateName((ITCStateNumber) newstate));
      else bs.Printf("???");
      return;
    } 

    if (mTraceType == TTC_EW_StateChange) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      u8 newstate;
      if (1 == cbbs.Scanf("%c",&newstate)) bs.Printf("-> %s\n", getEWStateName((EWStateNumber) newstate));
      else bs.Printf("???");
      return;
    } 

    if (mTraceType == TTC_EW_AssignCenter) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      s8 cx,cy;
      u8 radius, active;
      if (4 == cbbs.Scanf("%c%c%c%c",&cx,&cy,&radius,&active))
        bs.Printf("%c@(%d,%d)+%d\n",
                  active?'a':'p',
                  (s32) cx, (s32) cy,
                  radius);
      else bs.Printf("???");
      return;
    } 

    if (mTraceType == TTC_EW_CircuitStateChange) {
      CharBufferByteSource cbbs = mData.AsByteSource();

      u8 itcdir6, oldcs, newcs;
      if (3 == cbbs.Scanf("%c%c%c",&itcdir6,&oldcs,&newcs))
        bs.Printf("%s CS_%s -> CS_%s\n",
                  itcdir6 == 0xff ? "--" : getDir6Name(itcdir6),
                  getCircuitStateName((CircuitState) oldcs),
                  getCircuitStateName((CircuitState) newcs)
                  );
      else bs.Printf("???");
      return;
    } 
    
    if (mTraceType == TTC_Log_LogTrace) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      bs.Printf("%<\n",&cbbs);
      return;
    } 

    if (false) { }
    else if (mTraceType == TTC_ITC_PacketIn) bs.Printf("<");
    else if (mTraceType == TTC_ITC_PacketOut) bs.Printf(">");
    else bs.Printf(" ");
      
    reportPacketAnalysis(mData,bs);
    CharBufferByteSource cbbs = payloadRead();
    u32 plen = cbbs.GetLength();
    bs.Printf(" +%d",plen);
    bs.Printf("\n");
  }

  void TraceLoggerInMemory::dump(const char * path) {
    FILE * file = fopen(path,"w");
    if (file == 0) 
      STDOUT.Printf("Can't dump to '%s': %s\n",
                    path, strerror(errno));
    else {
      FileByteSink fbs(file);
      mTraceBuffers[1-mCurBuf].AppendTo(fbs); // Older first
      mTraceBuffers[mCurBuf].AppendTo(fbs); // Then current
      fbs.Close();
    }
  }

  void TraceLogger::log(ByteSink & bs, const Trace & evt) {
    bs.Printf("%c%c",
                TRACE_REC_START_BYTE1,
                TRACE_REC_START_BYTE2
                );
    evt.mLocalTimestamp.Print(bs);
    evt.getTraceAddress().print(bs);
    CharBufferByteSource cbbs = evt.payloadRead();
    u32 plen = cbbs.GetLength();
    MFM_API_ASSERT_STATE(plen < 256);
    bs.Printf("%c%c%<",
              evt.mTraceType,
              plen,
              &cbbs
              );
  }

  Trace * TraceLogReader::read(ByteSource& bs,
                               struct timespec basetime,
                               struct timespec timeoffset) {
    u8 byte1, byte2;
    if (2 != bs.Scanf("%c%c",
                      &byte1, &byte2))
      return 0;

    if (byte1 != TRACE_REC_START_BYTE1 ||
        byte2 != TRACE_REC_START_BYTE2) {
      LOG.Error("Invalid trace rec header, wanted 0x%02x%02x found 0x%02x%02x",
                TRACE_REC_START_BYTE1, TRACE_REC_START_BYTE2,
                byte1, byte2);
      return 0;
    }

    UniqueTime tmpTime;
    if (!tmpTime.Scan(bs)) {
      return 0;
    }
    TraceAddress tmpAddr;
    if (!tmpAddr.read(bs)) {
      return 0;
    }
    
    u32 traceType;
    u8 plen;

    if (2 != bs.Scanf("%c%c",
                      &traceType,
                      &plen))
      return 0;
    OString256 tmpData;
    for (u32 i = 0; i < plen; ++i) {
      s32 ch = bs.Read();
      if (ch < 0) return 0;
      tmpData.WriteByte((u8) ch);
    }

    UniqueTime reltime(tmpTime
                       - UniqueTime(basetime,0)
                       + UniqueTime(timeoffset, 0));
                       
    Trace * ret = new Trace(traceType, reltime);
    ret->setTraceAddress(tmpAddr);

    CharBufferByteSource cbbs = tmpData.AsByteSource();
    ret->payloadWrite().Copy(cbbs);

    s32 tag;
    if (ret->reportSyncIfAny(tag)) {
      ret->setTag(tag);
    }

    return ret;
  }

  bool Trace::reportSyncIfAny(s32 & store) const {
    // CURRENTLY THERE'S SYNC IN THE FOLLOWING PLACES:
    // - All TFM traces
    // - POU/PIN T2ITC if byte1 is xitcByte1(XITC_ITC_CMD,SNUM)
    //   with SNUM == ITCSN_SHUT or ITCSN_OPEN
    //   FOR WHICH SEE ALSO T2PacketBuffer.cpp: asTagSync
    TraceAddress addr = getTraceAddress();
    u8 tt = mTraceType;
    if (addr.getMode() == TRACE_REC_MODE_T2TILE &&
        tt == TTC_Tile_TraceFileMarker) {
      CharBufferByteSource cbbs = mData.AsByteSource();
      s32 tag;
      if (1 != cbbs.Scanf("%l", &tag)) return false;
      if (tag == 0) return false; //??
      store = tag; // tag is already signed in mData
      return true;
    }

    if (addr.getMode() != TRACE_REC_MODE_ITCDIR6) return false;
    if (tt != TTC_ITC_PacketOut && tt != TTC_ITC_PacketIn) return false;
    u8 sn;
    if (!asITC(mData,&sn)) return false;
    if (sn != ITCSN_SHUT && sn != ITCSN_OPEN) return false;

    CharBufferByteSource cbbs = mData.AsByteSource();
    s32 tag;
    if (3 != cbbs.Scanf("%c%c%l", 0, 0, &tag)) return false;
    if (tag <= 0) return false; //??
    store = (tt == TTC_ITC_PacketOut) ? tag : -tag;
    return true;
  }

  u8 TraceAddress::getITCDir6() const {
    MFM_API_ASSERT_STATE(getMode() == TRACE_REC_MODE_ITCDIR6);
    return mArg1;
  }

  u8 TraceAddress::getITCState() const {
    MFM_API_ASSERT_STATE(getMode() == TRACE_REC_MODE_ITCDIR6);
    return mArg2;
  }

}
