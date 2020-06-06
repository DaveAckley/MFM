#include "Trace.h"
#include "TraceTypes.h"
#include "T2Tile.h"
#include "T2ITC.h"
#include "T2EventWindow.h"
               
namespace MFM {
  ///// STATIC MEMBER DEFINITIONS

  ////// TraceAddress
  TraceAddress::TraceAddress()
    : mAddrMode(TRACE_REC_MODE_ILLEGAL) , mArg1(0) , mArg2(0) { }
  TraceAddress::TraceAddress(const TraceAddress & oth)
    : mAddrMode(oth.mAddrMode) , mArg1(oth.mArg1) , mArg2(oth.mArg2) { }
  TraceAddress::TraceAddress(const T2Tile & tile)
    : mAddrMode(TRACE_REC_MODE_T2TILE) , mArg1(0) , mArg2(0) { }
  TraceAddress::TraceAddress(const T2ITC & itc)
    : mAddrMode(TRACE_REC_MODE_ITCDIR6) , mArg1(itc.mDir6) , mArg2(itc.getITCSN()) { }
  TraceAddress::TraceAddress(const T2EventWindow & ew) {
    if (ew.isInActiveState()) {
      mAddrMode = TRACE_REC_MODE_EWACTIV;
      mArg1 = ew.slotNum();
      mArg2 = ew.getEWSN();
    } else {
      mAddrMode = TRACE_REC_MODE_EWPASIV;
      const CircuitInfo & ci = ew.getPassiveCircuitInfo();
      mArg1 = ci.mITC->mDir6;
      mArg2 = ci.mCircuitNum;
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
    case TRACE_REC_MODE_ILLEGAL: return "illegal";
    case TRACE_REC_MODE_T2TILE : return "t";
    case TRACE_REC_MODE_ITCDIR6: return "i";
    case TRACE_REC_MODE_EWACTIV: return "a";
    case TRACE_REC_MODE_EWPASIV: return "p";
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
      bs.Printf("%02d",mArg1);
      bs.Printf("-%s",getEWStateName((EWStateNumber) mArg2));
      break;
    case TRACE_REC_MODE_EWPASIV:
      bs.Printf("%s",getDir6Name(mArg1));
      bs.Printf("#%x",mArg2);
      break;
    }
  }

  void Trace::printPretty(ByteSink & bs, bool includeTime) const {
    if (includeTime) {
      mLocalTimestamp.printPretty(bs);
      bs.Printf(" ");
    }
    //    bs.Printf("%s ",getTraceTypeBrief((TraceTypeCode) mTraceType));
    getTraceAddress().printPretty(bs);
    bs.Printf(" ");
    /*
    s32 tag = getTag();
    if (tag != 0) {
      bs.Printf("%c%08x",
                tag > 0 ? 'S' : 'R',
                tag > 0 ? tag : -tag);
    }
    */
    if (false) { }
    else if (mTraceType == TTC_ITC_PacketIn) bs.Printf("<");
    else if (mTraceType == TTC_ITC_PacketOut) bs.Printf(">");
    else bs.Printf(" ");
      
    T2Packet::reportPacketAnalysis(mData,bs);
    CharBufferByteSource cbbs = payloadRead();
    u32 plen = cbbs.GetLength();
    bs.Printf(" +%d",plen);
    /*
    for (u32 i = 0; i < plen; ++i) {
      if (i%8 == 0) bs.Printf(" ");
      bs.Printf("%02x",cbbs.Read());
    }
    */
    bs.Printf("\n");
  }


  void TraceLogger::log(const Trace & evt) {
    mFBS.Printf("%c%c",
                TRACE_REC_START_BYTE1,
                TRACE_REC_START_BYTE2
                );
    evt.mLocalTimestamp.Print(mFBS);
    evt.getTraceAddress().print(mFBS);
    CharBufferByteSource cbbs = evt.payloadRead();
    u32 plen = cbbs.GetLength();
    MFM_API_ASSERT_STATE(plen < 256);
    mFBS.Printf("%c%c%<",
                evt.mTraceType,
                plen,
                &cbbs
                );
  }

  Trace * TraceLogReader::read(struct timespec timeOffset) {
    u8 byte1, byte2;
    if (2 != mBS.Scanf("%c%c",
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
    if (!tmpTime.Scan(mBS)) {
      return 0;
    }
    TraceAddress tmpAddr;
    if (!tmpAddr.read(mBS)) {
      return 0;
    }
    
    u32 traceType;
    //    u8 tagSrc;
    //    u32 utag;
    u8 plen;

    if (4 != mBS.Scanf("%D%c%D%c",
                       &traceType,
                       0, // XXXX NOT USING EXTRACTED TAG// &tagSrc,
                       0, // XXXX NOT USING EXTRACTED TAG// &utag,
                       &plen))
      return 0;
    OString256 tmpData;
    for (u32 i = 0; i < plen; ++i) {
      s32 ch = mBS.Read();
      if (ch < 0) return 0;
      tmpData.WriteByte((u8) ch);
    }

    if (!mHaveFirst) {
      mFirstTimespec = tmpTime.getTimespec();
      mHaveFirst = true;
    }
    UniqueTime reltime(tmpTime
                       - UniqueTime(mFirstTimespec,0)
                       + UniqueTime(timeOffset,0));
    Trace * ret = new Trace(traceType, reltime);
    ret->setTraceAddress(tmpAddr);
    // XXXX NOT USING 'PRE-EXTRACTED' TAG INFO FROM TRACES
    // ret->setTag(tagSrc == 'S' ? (s32) utag : -(s32) utag);
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
    // POU T2ITC SHUT
    // PIN T2ITC DRAIN
    // POU T2ITC OPEN
    // PIN T2ITC OPEN

    TraceAddress addr = getTraceAddress();
    if (addr.getMode() != TRACE_REC_MODE_ITCDIR6) return false;
    u8 tt = mTraceType;
    u8 itcState = addr.getITCState();
    bool stdSync = false;
    if (false) { }
    else if (tt == TTC_ITC_PacketOut  && itcState == ITCSN_SHUT) stdSync = true;
    else if (tt == TTC_ITC_PacketIn   && itcState == ITCSN_DRAIN) stdSync = true;
    else if (tt == TTC_ITC_PacketOut  && itcState == ITCSN_OPEN) stdSync = true;
    else if (tt == TTC_ITC_PacketIn   && itcState == ITCSN_OPEN) stdSync = true;
    if (!stdSync) return false;
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
